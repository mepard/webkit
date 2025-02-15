/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "config.h"
#import "NetworkSession.h"

#if USE(NETWORK_SESSION)

#import <WebCore/AuthenticationChallenge.h>
#import <WebCore/ResourceRequest.h>
#import <wtf/MainThread.h>

namespace WebKit {

NetworkDataTask::NetworkDataTask(NetworkSession& session, NetworkDataTaskClient& client, const WebCore::ResourceRequest& requestWithCredentials, WebCore::StoredCredentials storedCredentials)
    : m_failureTimer(*this, &NetworkDataTask::failureTimerFired)
    , m_session(session)
    , m_client(client)
{
    ASSERT(isMainThread());
    
    if (!requestWithCredentials.url().isValid()) {
        scheduleFailure(InvalidURLFailure);
        return;
    }
    
    if (!portAllowed(requestWithCredentials.url())) {
        scheduleFailure(BlockedFailure);
        return;
    }
    
    auto request = requestWithCredentials;
    m_user = request.url().user();
    m_password = request.url().pass();
    request.removeCredentials();
    
    if (storedCredentials == WebCore::AllowStoredCredentials)
        m_task = [m_session.m_sessionWithCredentialStorage dataTaskWithRequest:request.nsURLRequest(WebCore::UpdateHTTPBody)];
    else
        m_task = [m_session.m_sessionWithoutCredentialStorage dataTaskWithRequest:request.nsURLRequest(WebCore::UpdateHTTPBody)];
    
    ASSERT(!m_session.m_dataTaskMap.contains(taskIdentifier()));
    m_session.m_dataTaskMap.add(taskIdentifier(), this);
}

NetworkDataTask::~NetworkDataTask()
{
    if (m_task) {
        ASSERT(m_session.m_dataTaskMap.contains(taskIdentifier()));
        ASSERT(m_session.m_dataTaskMap.get(taskIdentifier()) == this);
        ASSERT(isMainThread());
        m_session.m_dataTaskMap.remove(taskIdentifier());
    }
}

void NetworkDataTask::scheduleFailure(FailureType type)
{
    ASSERT(type != NoFailure);
    m_scheduledFailureType = type;
    m_failureTimer.startOneShot(0);
}

void NetworkDataTask::failureTimerFired()
{
    RefPtr<NetworkDataTask> protect(this);
    
    switch (m_scheduledFailureType) {
    case BlockedFailure:
        m_scheduledFailureType = NoFailure;
        client().wasBlocked();
        return;
    case InvalidURLFailure:
        m_scheduledFailureType = NoFailure;
        client().cannotShowURL();
        return;
    case NoFailure:
        ASSERT_NOT_REACHED();
        break;
    }
    ASSERT_NOT_REACHED();
}

bool NetworkDataTask::tryPasswordBasedAuthentication(const WebCore::AuthenticationChallenge& challenge, ChallengeCompletionHandler completionHandler)
{
    if (!challenge.protectionSpace().isPasswordBased())
        return false;
    
    if (!m_user.isNull() && !m_password.isNull()) {
        completionHandler(AuthenticationChallengeDisposition::UseCredential, WebCore::Credential(m_user, m_password, WebCore::CredentialPersistenceForSession));
        m_user = String();
        m_password = String();
        return true;
    }
    
    if (!challenge.proposedCredential().isEmpty() && !challenge.previousFailureCount()) {
        completionHandler(AuthenticationChallengeDisposition::UseCredential, challenge.proposedCredential());
        return true;
    }
    
    return false;
}

void NetworkDataTask::cancel()
{
    [m_task cancel];
}

void NetworkDataTask::resume()
{
    if (m_scheduledFailureType != NoFailure)
        m_failureTimer.startOneShot(0);
    [m_task resume];
}

void NetworkDataTask::suspend()
{
    if (m_failureTimer.isActive())
        m_failureTimer.stop();
    [m_task suspend];
}

auto NetworkDataTask::taskIdentifier() -> TaskIdentifier
{
    return [m_task taskIdentifier];
}
}

#endif

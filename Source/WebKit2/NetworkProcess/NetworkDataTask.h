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

#ifndef NetworkDataTask_h
#define NetworkDataTask_h

#include <WebCore/FrameLoaderTypes.h>
#include <WebCore/ResourceHandleTypes.h>
#include <WebCore/Timer.h>
#include <wtf/RetainPtr.h>
#include <wtf/text/WTFString.h>

#if PLATFORM(COCOA)
OBJC_CLASS NSURLSessionDataTask;
#endif

namespace WebCore {
class AuthenticationChallenge;
class Credential;
class ResourceError;
class ResourceRequest;
class ResourceResponse;
class SharedBuffer;
}

namespace WebKit {

class NetworkSession;
class PendingDownload;

enum class AuthenticationChallengeDisposition {
    UseCredential,
    PerformDefaultHandling,
    Cancel,
    RejectProtectionSpace
};
    
typedef std::function<void(const WebCore::ResourceRequest&)> RedirectCompletionHandler;
typedef std::function<void(AuthenticationChallengeDisposition, const WebCore::Credential&)> ChallengeCompletionHandler;
typedef std::function<void(WebCore::PolicyAction)> ResponseCompletionHandler;

class NetworkDataTaskClient {
public:
    virtual void willPerformHTTPRedirection(const WebCore::ResourceResponse&, const WebCore::ResourceRequest&, RedirectCompletionHandler) = 0;
    virtual void didReceiveChallenge(const WebCore::AuthenticationChallenge&, ChallengeCompletionHandler) = 0;
    virtual void didReceiveResponseNetworkSession(const WebCore::ResourceResponse&, ResponseCompletionHandler) = 0;
    virtual void didReceiveData(RefPtr<WebCore::SharedBuffer>&&) = 0;
    virtual void didCompleteWithError(const WebCore::ResourceError&) = 0;
    virtual void didBecomeDownload() = 0;
    virtual void didSendData(uint64_t totalBytesSent, uint64_t totalBytesExpectedToSend) = 0;
    virtual void wasBlocked() = 0;
    virtual void cannotShowURL() = 0;
    
    virtual ~NetworkDataTaskClient() { }
};

class NetworkDataTask : public RefCounted<NetworkDataTask> {
    friend class NetworkSession;
public:
    static Ref<NetworkDataTask> create(NetworkSession& session, NetworkDataTaskClient& client, const WebCore::ResourceRequest& request, WebCore::StoredCredentials storedCredentials)
    {
        return adoptRef(*new NetworkDataTask(session, client, request, storedCredentials));
    }
    
    void suspend();
    void cancel();
    void resume();
    
    typedef uint64_t TaskIdentifier;
    TaskIdentifier taskIdentifier();
    
    ~NetworkDataTask();
    
    NetworkDataTaskClient& client() { return m_client; }
    
    DownloadID pendingDownloadID() { return m_pendingDownloadID; }
    PendingDownload* pendingDownload() { return m_pendingDownload; }
    void setPendingDownloadID(DownloadID downloadID)
    {
        ASSERT(!m_pendingDownloadID.downloadID());
        ASSERT(downloadID.downloadID());
        m_pendingDownloadID = downloadID;
    }
    void setPendingDownload(PendingDownload& pendingDownload)
    {
        ASSERT(!m_pendingDownload);
        m_pendingDownload = &pendingDownload;
    }
    bool tryPasswordBasedAuthentication(const WebCore::AuthenticationChallenge&, ChallengeCompletionHandler);
    
private:
    NetworkDataTask(NetworkSession&, NetworkDataTaskClient&, const WebCore::ResourceRequest&, WebCore::StoredCredentials);
    
    enum FailureType {
        NoFailure,
        BlockedFailure,
        InvalidURLFailure
    };
    FailureType m_scheduledFailureType { NoFailure };
    WebCore::Timer m_failureTimer;
    void failureTimerFired();
    void scheduleFailure(FailureType);
    
    NetworkSession& m_session;
    NetworkDataTaskClient& m_client;
    PendingDownload* m_pendingDownload { nullptr };
    DownloadID m_pendingDownloadID;
    String m_user;
    String m_password;
#if PLATFORM(COCOA)
    RetainPtr<NSURLSessionDataTask> m_task;
#endif
};

}

#endif

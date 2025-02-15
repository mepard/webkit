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

#import "WKFoundation.h"

#if WK_API_ENABLED

#if ENABLE(REMOTE_INSPECTOR)

#import "APIAutomationClient.h"
#import "WeakObjCPtr.h"
#import <JavaScriptCore/RemoteInspector.h>

@class WKProcessPool;

@protocol _WKAutomationDelegate;

namespace WebKit {

class AutomationClient final : public API::AutomationClient, Inspector::RemoteInspector::Client {
public:
    explicit AutomationClient(WKProcessPool *, id <_WKAutomationDelegate>);
    virtual ~AutomationClient();

private:
    // API::AutomationClient
    virtual bool allowsRemoteAutomation(WebProcessPool*) override { return remoteAutomationAllowed(); }
    virtual void requestAutomationSession(WebProcessPool*) override { requestAutomationSession(); }

    // RemoteInspector::Client
    virtual bool remoteAutomationAllowed() const override;
    virtual void requestAutomationSession() override;

    WKProcessPool *m_processPool;
    WeakObjCPtr<id <_WKAutomationDelegate>> m_delegate;

    struct {
        bool allowsRemoteAutomation : 1;
        bool requestAutomationSession : 1;
    } m_delegateMethods;
};

} // namespace WebKit

#endif // ENABLE(REMOTE_INSPECTOR)

#endif // WK_API_ENABLED

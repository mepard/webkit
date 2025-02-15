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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef FTLExceptionTarget_h
#define FTLExceptionTarget_h

#include "DFGCommon.h"

#if ENABLE(FTL_JIT) && FTL_USES_B3

#include "CCallHelpers.h"
#include "FTLOSRExitHandle.h"
#include <wtf/Box.h>
#include <wtf/ThreadSafeRefCounted.h>

namespace JSC { namespace FTL {

class State;

class ExceptionTarget : public ThreadSafeRefCounted<ExceptionTarget> {
public:
    ~ExceptionTarget();

    // It's OK to call this during linking, but not any sooner.
    CodeLocationLabel label(LinkBuffer&);

    // Or, you can get a JumpList at any time. Anything you add to this JumpList will be linked to
    // the target's label.
    Box<CCallHelpers::JumpList> jumps(CCallHelpers&);
    
private:
    friend class PatchpointExceptionHandle;

    ExceptionTarget(bool isDefaultHandler, Box<CCallHelpers::Label>, RefPtr<OSRExitHandle>);
    
    bool m_isDefaultHandler;
    Box<CCallHelpers::Label> m_defaultHandler;
    RefPtr<OSRExitHandle> m_handle;
};

} } // namespace JSC::FTL

#endif // ENABLE(FTL_JIT) && FTL_USES_B3

#endif // FTLExceptionTarget_h


/******************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Debug Bridge.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
******************************************************************************/
#ifndef SCOPEGUARD_H
#define SCOPEGUARD_H

#include <functional>

class ScopeGuard {
public:
    template<class Callable>
    ScopeGuard(Callable &&callable)
        : m_function(std::forward<Callable>(callable)) {}

    ScopeGuard(ScopeGuard &&other)
        : m_function(std::move(other.m_function)) {
        other.m_function = nullptr;
    }

    ~ScopeGuard() {
        if (m_function)
            m_function();
    }

    void dismiss() throw() {
        m_function = nullptr;
    }

    ScopeGuard(const ScopeGuard&) = delete;
    void operator = (const ScopeGuard&) = delete;

private:
    std::function<void()> m_function;
};

#endif // SCOPEGUARD_H

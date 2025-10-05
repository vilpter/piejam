// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/entity_id_hash.h>
#include <piejam/pimpl.h>
#include <piejam/redux/subscriber.h>
#include <piejam/runtime/state_access.h>
#include <piejam/runtime/subscriber.h>

#include <QObject>
#include <QTimerEvent>

#include <boost/assert.hpp>
#include <boost/core/ignore_unused.hpp>

#include <functional>

namespace piejam::gui::model
{

class SubscribableModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(
        bool subscribed READ subscribed WRITE setSubscribed NOTIFY
            subscribedChanged)

public:
    auto subscribed() const -> bool
    {
        return m_subscribed;
    }

    void setSubscribed(bool subs)
    {
        if (m_subscribed != subs)
        {
            if (subs)
            {
                subscribe();
            }
            else
            {
                unsubscribe();
            }

            m_subscribed = subs;

            emit subscribedChanged();
        }
    }

signals:
    void subscribedChanged();

protected:
    SubscribableModel(runtime::state_access state_access)
        : m_state_access(state_access)
    {
    }

    auto state_access() const noexcept -> runtime::state_access const&
    {
        return m_state_access;
    }

    auto dispatch(runtime::action const& a) const
    {
        m_state_access.dispatch(a);
    }

    template <class Value>
    auto observe_once(runtime::selector<Value> const& sel)
    {
        return m_state_access.observe_once(sel);
    }

    template <class Value, class Handler>
    void observe(runtime::selector<Value> sel, Handler&& h)
    {
        m_subs.emplace_back(
            m_state_access.observe(std::move(sel), std::forward<Handler>(h)));
    }

    template <class F>
    void requestUpdates(std::chrono::milliseconds t, F&& f)
    {
        BOOST_ASSERT(!m_updateTimerId);
        m_requestUpdate = std::forward<F>(f);
        m_updateTimerId = QObject::startTimer(t);
    }

    virtual void onSubscribe() = 0;

    auto attachChildModel(SubscribableModel& child)
    {
        return QObject::connect(
            this,
            &SubscribableModel::subscribedChanged,
            &child,
            [this, &child]() { child.setSubscribed(subscribed()); });
    }

    template <class Model, class... Args>
    auto makeModel(Args&&... args)
    {
        return make_pimpl<Model>(m_state_access, std::forward<Args>(args)...);
    }

    template <class Model, class... Args>
    auto makeChildModel(Args&&... args)
    {
        auto child = makeModel<Model>(std::forward<Args>(args)...);
        attachChildModel(*child);
        return child;
    }

    template <class ParameterT, class ParameterIdT>
    void makeParameter(
        std::unique_ptr<ParameterT>& param,
        ParameterIdT const param_id)
    {
        param = std::make_unique<ParameterT>(m_state_access, param_id);
    }

    template <class StreamT, class StreamIdT>
    void makeStream(std::unique_ptr<StreamT>& stream, StreamIdT const stream_id)
    {
        stream = std::make_unique<StreamT>(m_state_access, stream_id);
        attachChildModel(*stream);
    }

private:
    void subscribe()
    {
        onSubscribe();
    }

    void unsubscribe()
    {
        m_subs.clear();

        if (m_updateTimerId != 0)
        {
            QObject::killTimer(m_updateTimerId);
            m_updateTimerId = 0;
        }
    }

    void timerEvent(QTimerEvent* const event) final
    {
        BOOST_ASSERT(event->timerId() == m_updateTimerId);
        BOOST_ASSERT(m_requestUpdate);
        boost::ignore_unused(event);
        m_requestUpdate();
    }

    bool m_subscribed{};

    runtime::state_access m_state_access;
    std::vector<redux::subscription> m_subs;

    int m_updateTimerId{};
    std::function<void()> m_requestUpdate;
};

} // namespace piejam::gui::model

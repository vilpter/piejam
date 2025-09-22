// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/entity_id_hash.h>
#include <piejam/redux/subscriptions_manager.h>
#include <piejam/runtime/store_dispatch.h>
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
    SubscribableModel(
            runtime::store_dispatch store_dispatch,
            runtime::subscriber& state_change_subscriber)
        : m_store_dispatch(store_dispatch)
        , m_state_change_subscriber(state_change_subscriber)
    {
    }

    auto state_change_subscriber() const noexcept -> runtime::subscriber&
    {
        return m_state_change_subscriber;
    }

    auto dispatch() const noexcept -> runtime::store_dispatch
    {
        return m_store_dispatch;
    }

    auto dispatch(runtime::action const& a) const
    {
        m_store_dispatch(a);
    }

    template <class Value>
    auto observe_once(runtime::selector<Value> const& sel)
    {
        return m_state_change_subscriber.observe_once(sel);
    }

    template <class Value, class Handler>
    void observe(runtime::selector<Value> sel, Handler&& h)
    {
        observe(m_subs_id, std::move(sel), std::forward<Handler>(h));
    }

    template <class Value, class Handler>
    void
    observe(runtime::subscription_id subs_id,
            runtime::selector<Value> sel,
            Handler&& h)
    {
        m_subs.observe(
                subs_id,
                m_state_change_subscriber,
                std::move(sel),
                std::forward<Handler>(h));
    }

    void unobserve(runtime::subscription_id subs_id)
    {
        m_subs.erase(subs_id);
    }

    template <class F>
    void requestUpdates(std::chrono::milliseconds t, F&& f)
    {
        BOOST_ASSERT(!m_updateTimerId);
        m_requestUpdate = std::forward<F>(f);
        m_updateTimerId = QObject::startTimer(t);
    }

    virtual void onSubscribe()
    {
    }

    virtual void onUnsubscribe()
    {
    }

    auto connectSubscribableChild(SubscribableModel& child)
    {
        return QObject::connect(
                this,
                &SubscribableModel::subscribedChanged,
                &child,
                [this, &child]() { child.setSubscribed(subscribed()); });
    }

    template <class ParameterT, class ParameterIdT>
    void makeParameter(
            std::unique_ptr<ParameterT>& param,
            ParameterIdT const param_id)
    {
        param = std::make_unique<ParameterT>(
                dispatch(),
                this->state_change_subscriber(),
                param_id);
    }

    template <class StreamT, class StreamIdT>
    void makeStream(std::unique_ptr<StreamT>& stream, StreamIdT const stream_id)
    {
        stream = std::make_unique<StreamT>(
                dispatch(),
                this->state_change_subscriber(),
                stream_id);
        connectSubscribableChild(*stream);
    }

private:
    void subscribe()
    {
        onSubscribe();
    }

    void unsubscribe()
    {
        onUnsubscribe();

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

    runtime::store_dispatch m_store_dispatch;
    runtime::subscriber& m_state_change_subscriber;
    runtime::subscriptions_manager m_subs;
    runtime::subscription_id const m_subs_id{
            runtime::subscription_id::generate()};

    int m_updateTimerId{};
    std::function<void()> m_requestUpdate;
};

} // namespace piejam::gui::model

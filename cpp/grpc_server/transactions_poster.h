#ifndef SERVER_TRANSACTIONS_POSTER
#define SERVER_TRANSACTIONS_POSTER

#include <boost/signals2/connection.hpp>
#include <broadcast_queue.h>

#include "async_core/async_receiver_iface.h"
#include "async_core/async_vfo_iface.h"

namespace server
{

struct Transaction {
};

class TransactionsPoster
{
public:
    TransactionsPoster(core::AsyncReceiverIface::sptr rx);
    broadcast_queue::receiver<Transaction> subscribe();

private:
    void onReceiverEvent(core::ReceiverEvent&);
    void onVfoEvent(core::AsyncVfoIface::sptr, core::VfoEvent&);

private:
    struct VfoData {
        core::AsyncVfoIface::sptr vfo;
        boost::signals2::scoped_connection connection;
    };

    broadcast_queue::sender<Transaction> tx_sender;
    core::AsyncReceiverIface::sptr rx;
    boost::signals2::scoped_connection connection;
    // std::vector<VfoData>
};

} // namespace server

#endif

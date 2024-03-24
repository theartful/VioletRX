#include <functional>

#include "async_core/types.h"
#include "broadcast_queue.h"
#include "transactions_poster.h"

namespace server
{
static constexpr size_t QUEUE_SIZE = 128;

TransactionsPoster::TransactionsPoster(violetrx::AsyncReceiverIface::sptr rx) :
    tx_sender{QUEUE_SIZE}, rx{rx}
{
}

} // namespace server

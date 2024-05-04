#ifndef VIOLETRX_DSP_BUFFER_SINK
#define VIOLETRX_DSP_BUFFER_SINK

#include <memory>

#include <gnuradio/sync_block.h>

#include "broadcast_queue.h"

class buffer_sink : public gr::sync_block
{
public:
    using sptr = std::shared_ptr<buffer_sink>;

    struct Packet {
        static constexpr size_t PACKET_SIZE = 1024;
        float chan0[PACKET_SIZE];
        float chan1[PACKET_SIZE];
    };

private:
    struct private_construction_tag {
    };

public:
    static sptr make(size_t buffer_capacity);

    buffer_sink(size_t buffer_capacity, private_construction_tag);
    ~buffer_sink() override;

    int work(int noutput_items, gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;

    broadcast_queue::receiver<Packet> subscribe();

private:
    broadcast_queue::sender<Packet> sender;
    Packet current_packet;
    size_t current_packet_idx;
};

#endif // INDIGO_DSP_BUFFER_SINK

#include "buffer_sink.h"
#include <algorithm>

buffer_sink::sptr buffer_sink::make(size_t buffer_capacity)
{
    return gnuradio::make_block_sptr<buffer_sink>(buffer_capacity,
                                                  private_construction_tag{});
}

buffer_sink::buffer_sink(size_t buffer_capacity, private_construction_tag) :
    gr::sync_block("buffer_sink", gr::io_signature::make(2, 2, sizeof(float)),
                   gr::io_signature::make(0, 0, 0)),
    sender{buffer_capacity},
    current_packet_idx{0}
{
}

broadcast_queue::receiver<buffer_sink::Packet> buffer_sink::subscribe()
{
    return sender.subscribe();
}

int buffer_sink::work(int noutput_items, gr_vector_const_void_star& input_items,
                      gr_vector_void_star& /* output_items */)
{
    float* chan0 = (float*)input_items[0];
    float* chan1 = (float*)input_items[1];

    int nread = 0;
    while (nread < noutput_items) {
        int packet_samps_num = Packet::PACKET_SIZE - current_packet_idx;
        size_t to_read = std::min(packet_samps_num, noutput_items - nread);

        std::memcpy(current_packet.chan0 + current_packet_idx, chan0 + nread,
                    to_read * sizeof(float));

        std::memcpy(current_packet.chan1 + current_packet_idx, chan1 + nread,
                    to_read * sizeof(float));

        current_packet_idx += to_read;

        if (current_packet_idx == Packet::PACKET_SIZE) {
            sender.push(current_packet);
            current_packet_idx = 0;
        }

        nread += to_read;
    }

    return noutput_items;
}

buffer_sink::~buffer_sink() {}

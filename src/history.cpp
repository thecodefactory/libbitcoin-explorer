#include <bitcoin/bitcoin.hpp>
#include <obelisk/client/interface.hpp>
#include "config.hpp"
#include "util.hpp"

using namespace bc;
using std::placeholders::_1;
using std::placeholders::_2;

typedef std::vector<payment_address> payaddr_list;

bool stopped = false;

void history_fetched(const payment_address& payaddr,
    const std::error_code& ec, const blockchain::history_list& history)
{
    if (ec)
    {
        std::cerr << "history: Failed to fetch history: "
            << ec.message() << std::endl;
        return;
    }
    for (const auto& row: history)
    {
        std::cout << "Address: " << payaddr.encoded() << std::endl;
        std::cout << "  output: " << row.output
            << "  height: " << row.output_height << std::endl;
        std::cout << "  value:  " << row.value << std::endl;
        std::cout << "  spend:  ";
        if (row.spend.hash == null_hash)
        {
            std::cout << "Unspent";
        }
        else
        {
            std::cout << row.spend << "  height: " << row.spend_height;
        }
        std::cout << std::endl << std::endl;
    }
    stopped = true;
}

bool payaddr_from_stdin(payment_address& payaddr)
{
    if (!payaddr.set_encoded(read_stdin()))
    {
        std::cerr << "history: Invalid address." << std::endl;
        return false;
    }
    return true;
}

bool payaddr_from_argv(payaddr_list& payaddrs, int argc, char** argv)
{
    for (size_t i = 1; i < argc; ++i)
    {
        payment_address payaddr;
        if (!payaddr.set_encoded(argv[i]))
            return false;
    }
    return true;
}

int main(int argc, char** argv)
{
    config_map_type config;
    load_config(config);
    payaddr_list payaddrs;
    if (argc == 1)
    {
        payment_address payaddr;
        if (!payaddr_from_stdin(payaddr))
            return -1;
        payaddrs.push_back(payaddr);
    }
    else
    {
        if (!payaddr_from_argv(payaddrs, argc, argv))
            return -1;
    }
    fullnode_interface fullnode(config["service"]);
    for (const payment_address& payaddr: payaddrs)
        fullnode.fetch_history(payaddr,
            std::bind(history_fetched, payaddr, _1, _2));
    while (!stopped)
    {
        fullnode.update();
        sleep(0.1);
    }
    return 0;
}


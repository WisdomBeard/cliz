#include <iostream>
#include <sstream>
#include <string>
#include <optional>
#include <nlohmann/json.hpp>
#include <zmq.hpp>
#include <cppcodec/base64_rfc4648.hpp>
#include <cppcodec/hex_lower.hpp>
#include <argh.h>
#include <cctype>    // std::tolower
#include <algorithm> // std::equal
#include <tuple>
#include <exception>

using namespace std;
using json = nlohmann::json;

bool ichar_equals(char a, char b)
{
    return std::tolower(static_cast<unsigned char>(a)) ==
           std::tolower(static_cast<unsigned char>(b));
}

bool iequals(const std::string& a, const std::string& b)
{
    return std::equal(a.begin(), a.end(), b.begin(), b.end(), ichar_equals);
}

enum class RecvFilterType
{
    bin2str,
    bin2hex,
    bin2b64,
    json2json,
    cbor2json
};

enum class SendFilterType
{
    str2bin,
    hex2bin,
    b642bin,
    json2json,
    json2cbor
};

class Config
{
    protected:
        optional< bool >             isServer;
        optional< zmq::socket_type > socketType;
        optional< string >           zmqAddress;
        RecvFilterType               recvFilter;
        SendFilterType               sendFilter;
        bool                         verbose;

    public:
        Config(int argc, char** argv)
            : isServer()
            , socketType()
            , zmqAddress()
            , recvFilter(RecvFilterType::bin2str)
            , sendFilter(SendFilterType::str2bin)
            , verbose(false)
        {
            argh::parser cmdl(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);

            // Client or server + ZMQ address
            string zmqAddressStr;
            if ( (cmdl("listen_on") >> zmqAddressStr) )
            {
                IsServer(true);
            }
            else if ( (cmdl("connect_to") >> zmqAddressStr) )
            {
                IsServer(false);
            }
            else
            {
                cerr << "Exactly one of listen_on|connect_to shall be provided" << endl;
                return; // impossible to go further
            }
            ZmqAddress(zmqAddressStr);

            // ZMQ socket type
            string socketTypeStr;
            if ( (cmdl("as") >> socketTypeStr) )
            {
                SocketType(socketTypeStr);
            }
            else
            {
                cerr << "Missing socket type (use --as)" << endl;
            }

            // Filters
            string recvFilterStr;
            if ( (cmdl("on_recv") >> recvFilterStr) )
            {
                RecvFilter(recvFilterStr);
            }
            string sendFilterStr;
            if ( (cmdl("on_send") >> sendFilterStr) )
            {
                SendFilter(sendFilterStr);
            }

            // Verbose
            Verbose(cmdl["v"]);
        }

        bool IsValid() const
        {
            return (isServer && socketType && zmqAddress);
        }

        // Getters

        bool IsServer() const
        {
            return isServer.value();
        }

        zmq::socket_type SocketType() const
        {
            return socketType.value();
        }

        const char* SocketTypeName() const
        {
            switch (socketType.value())
            {
            case zmq::socket_type::req:
                return "req";
            case zmq::socket_type::rep:
                return "rep";
            case zmq::socket_type::pub:
                return "pub";
            case zmq::socket_type::sub:
                return "sub";
            default:
                throw runtime_error("Invalid socket type");
            }
        }

        const string& ZmqAddress() const
        {
            return zmqAddress.value();
        }

        RecvFilterType RecvFilter() const
        {
            return recvFilter;
        }

        SendFilterType SendFilter() const
        {
            return sendFilter;
        }

        bool Verbose() const
        {
            return verbose;
        }

        // Setters

        void IsServer(bool newValue)
        {
            isServer = newValue;
        }

        void SocketType(zmq::socket_type newValue)
        {
            socketType = newValue;
        }

        void SocketType(const string& newValue)
        {
            static const tuple<string, zmq::socket_type> Z_SOCKET_TYPE_NAMES[] = {
                {string{"req"}, zmq::socket_type::req},
                {string{"rep"}, zmq::socket_type::rep},
                {string{"pub"}, zmq::socket_type::pub},
                {string{"sub"}, zmq::socket_type::sub}
            };
            static const auto BEGIN = begin(Z_SOCKET_TYPE_NAMES);
            static const auto END = end(Z_SOCKET_TYPE_NAMES);

            const auto& pairIt = find_if(BEGIN, END, [&](const auto& typeTuple){
                return iequals(get<0>(typeTuple), newValue);
            });

            if (pairIt == END)
            {
                stringstream msg;
                msg << "Invalid socket type. Shall be one of ";
                auto sep = "";
                for (const auto& [typeTuple, _] : Z_SOCKET_TYPE_NAMES)
                {
                    msg << sep << typeTuple;
                    sep = "|";
                }
                msg << "." << endl;
                throw runtime_error(msg.str());
            }

            SocketType(get<1>(*pairIt));
        }

        void ZmqAddress(const string& newValue)
        {
            zmqAddress = newValue;
        }

        void ZmqAddress(string&& newValue)
        {
            zmqAddress = move(newValue);
        }

        void RecvFilter(RecvFilterType newValue)
        {
            recvFilter = newValue;
        }

        void RecvFilter(const string& newValue)
        {
            static const tuple<string, RecvFilterType> FILTER_TYPES[] = {
                {string{"bin2str"}, RecvFilterType::bin2str},
                {string{"bin2hex"}, RecvFilterType::bin2hex},
                {string{"bin2b64"}, RecvFilterType::bin2b64},
                {string{"json2json"}, RecvFilterType::json2json},
                {string{"cbor2json"}, RecvFilterType::cbor2json}
            };
            static const auto BEGIN = begin(FILTER_TYPES);
            static const auto END = end(FILTER_TYPES);

            const auto& pairIt = find_if(BEGIN, END, [&](const auto& typeTuple){
                return iequals(get<0>(typeTuple), newValue);
            });

            if (pairIt == END)
            {
                stringstream msg;
                msg << "Invalid recv filter type. Shall be one of ";
                auto sep = "";
                for (const auto& [typeTuple, _] : FILTER_TYPES)
                {
                    msg << sep << typeTuple;
                    sep = "|";
                }
                msg << "." << endl;
                throw runtime_error(msg.str());
            }

            RecvFilter(get<1>(*pairIt));
        }

        void SendFilter(SendFilterType newValue)
        {
            sendFilter = newValue;
        }

        void SendFilter(const string& newValue)
        {
            static const tuple<string, SendFilterType> FILTER_TYPES[] = {
                {string{"str2bin"}, SendFilterType::str2bin},
                {string{"hex2bin"}, SendFilterType::hex2bin},
                {string{"b642bin"}, SendFilterType::b642bin},
                {string{"json2json"}, SendFilterType::json2json},
                {string{"json2cbor"}, SendFilterType::json2cbor}
            };
            static const auto BEGIN = begin(FILTER_TYPES);
            static const auto END = end(FILTER_TYPES);

            const auto& pairIt = find_if(BEGIN, END, [&](const auto& typeTuple){
                return iequals(get<0>(typeTuple), newValue);
            });

            if (pairIt == END)
            {
                cerr << "Invalid send filter type. Shall be one of ";
                auto sep = "";
                for (const auto& [typeTuple, _] : FILTER_TYPES)
                {
                    cerr << sep << typeTuple;
                    sep = "|";
                }
                cerr << "." << endl;
                return; // failure
            }

            SendFilter(get<1>(*pairIt));
        }

        void Verbose(bool newValue)
        {
            verbose = newValue;
        }
};

void ListenToStdin(zmq::socket_t* sock, Config* config, optional<unsigned int> amount = {})
{
    zmq::message_t msg;
    bool forever = !amount;
    unsigned int i = 0;
    if (amount)
    {
        i = *amount;
    }

    for (string line ; forever || i > 0 ; --i)
    {
        if (config->Verbose())
        {
            cout << ">>> ";
            cout.flush();
        }

        if (!cin || !getline(cin, line))
        {
            return;
        }

        switch (config->SendFilter())
        {
            case SendFilterType::str2bin :
                msg = zmq::message_t(move(line));
                break;
            case SendFilterType::hex2bin :
                msg = zmq::message_t(cppcodec::hex_lower::decode(line));
                break;
            case SendFilterType::b642bin :
                msg = zmq::message_t(cppcodec::base64_rfc4648::decode(line));
                break;
            case SendFilterType::json2json :
                msg = zmq::message_t(json::parse(line).dump());
                break;
            case SendFilterType::json2cbor :
                msg = zmq::message_t(json::to_cbor(json::parse(line)));
                break;
            default:
                throw runtime_error("Invalid send filter type");
        }
        sock->send(msg, zmq::send_flags::dontwait);
    }
}

void ListenToSocket(zmq::socket_t* sock, Config* config, optional<unsigned int> amount = {})
{
    zmq::message_t msg;
    bool forever = !amount;
    unsigned int i = 0;
    if (amount)
    {
        i = *amount;
    }

    for ( ; forever || i > 0 ; --i)
    {
        if (config->Verbose())
        {
            cout << "... ";
            cout.flush();
        }

        (void) sock->recv(msg);
        switch (config->RecvFilter())
        {
            case RecvFilterType::bin2str :
                cout << msg.to_string_view() << endl;
                break;
            case RecvFilterType::bin2hex :
                cout <<  cppcodec::hex_lower::encode(msg.data<unsigned char>(), msg.size()) << endl;
                break;
            case RecvFilterType::bin2b64 :
                cout <<  cppcodec::base64_rfc4648::encode(msg.data<unsigned char>(), msg.size()) << endl;
                break;
            case RecvFilterType::json2json :
                cout << json::parse(msg.to_string_view()).dump(2) << endl;
                break;
            case RecvFilterType::cbor2json :
                cout << json::from_cbor(msg.to_string_view()).dump(2) << endl;
                break;
            default:
                throw runtime_error("Invalid recv filter type");
        }
    }
}

int main(int argc, char** argv)
{
    try
    {
        Config config{argc, argv};

        if (!config.IsValid())
        {
            return 1; // messages were already printed, no need to print more
        }

        zmq::context_t ctx;
        zmq::socket_t sock(ctx, config.SocketType());
        if (config.IsServer())
        {
            if (config.Verbose())
            {
                cout << "Listening on " << config.ZmqAddress() << " as " << config.SocketTypeName() << "..." << endl;
            }
            sock.bind(config.ZmqAddress());
        }
        else
        {
            if (config.Verbose())
            {
                cout << "Connecting to " << config.ZmqAddress() << " as " << config.SocketTypeName() << "..." << endl;
            }
            sock.connect(config.ZmqAddress());
        }

        switch (config.SocketType())
        {
        case zmq::socket_type::pub:
        {
            ListenToStdin(&sock, &config);
            break;
        }
        case zmq::socket_type::sub:
        {
            sock.set(zmq::sockopt::subscribe, "");
            ListenToSocket(&sock, &config);
            break;
        }
        case zmq::socket_type::req:
        {
            while (true)
            {
                ListenToStdin(&sock, &config, 1);
                ListenToSocket(&sock, &config, 1);
            }
            break;
        }
        case zmq::socket_type::rep:
        {
            while (true)
            {
                ListenToSocket(&sock, &config, 1);
                ListenToStdin(&sock, &config, 1);
            }
            break;
        }
        default:
            throw runtime_error("Invalid socket type");
        }

        // Cease any blocking operations in progress.
        ctx.shutdown();
        // Do a shutdown, if needed and destroy the context.
        ctx.close();
    }
    catch (exception& e)
    {
        cerr << e.what() << endl;
        return 1;
    }

    return 0;
}

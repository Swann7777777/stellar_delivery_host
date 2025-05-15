#include <unordered_set>
#include <enet/enet.h>
#include <iostream>
#include <string>
#include <vector>
#include <random>


class planet_class {
public:
    int x;
    int y;
    int size;
    int index;
};





int main() {




    if (enet_initialize() != 0) {
        std::cout << "Enet could not initialize !" << std::endl;
        return -1;
    }
    atexit(enet_deinitialize);




    ENetAddress address;
    ENetHost* server;
    ENetEvent enet_event;

    address.host = ENET_HOST_ANY;
    address.port = 25565;


    server = enet_host_create(&address, 32, 1, 0, 0);


    if (!server) {
        std::cout << "Failed to create an Enet host !" << std::endl;
        return -1;
    }








    std::random_device rd;
    std::mt19937 gen(rd());


    std::uniform_int_distribution<> pos_dist(-30000000, 30000000);
    std::uniform_int_distribution<> size_dist(1000, 10000);
    std::uniform_int_distribution<> asset_dist(0, 2);





    std::vector<planet_class> planets;
    for (int i = 0; i < 100000; i++) {
        planets.push_back({ pos_dist(gen), pos_dist(gen), size_dist(gen), asset_dist(gen) });
    }


    size_t planet_size = planets.size() * sizeof(planet_class);
    char* planet_buffer = new char[planet_size];

    memcpy(planet_buffer, planets.data(), planet_size);

    ENetPacket* planet_packet;




    std::unordered_set<ENetPeer*> clients;

    std::string client_name;
    char* clientNameCopy;

    while (true) {
        while (enet_host_service(server, &enet_event, 1) > 0) {
            switch (enet_event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                std::cout << "Client connected from " << enet_event.peer->address.host << ":" << enet_event.peer->address.port << std::endl;



                client_name = "Client_" + std::to_string(enet_event.peer->address.host);
                clientNameCopy = new char[client_name.length() + 1];
                std::copy(client_name.begin(), client_name.end(), clientNameCopy);
                clientNameCopy[client_name.length()] = '\0';
                enet_event.peer->data = clientNameCopy;



                clients.insert(enet_event.peer);




                planet_packet = enet_packet_create(planet_buffer, planet_size, ENET_PACKET_FLAG_RELIABLE);
                if (!planet_packet) {
                    std::cout << "Failed to create packet!" << std::endl;
                    break;
                }

                if (enet_peer_send(enet_event.peer, 0, planet_packet) != 0) {
                    std::cout << "Failed to send packet!" << std::endl;
                }
                else {
                    std::cout << "Planet packet sent to the client." << std::endl;
                }

                break;
















            case ENET_EVENT_TYPE_RECEIVE:



                for (ENetPeer* peer : clients) {
                    if (peer != enet_event.peer) {
                        ENetPacket* position_packet = enet_packet_create(
                            enet_event.packet->data, enet_event.packet->dataLength, ENET_PACKET_FLAG_RELIABLE
                        );
                        if (position_packet) {
                            enet_peer_send(peer, 0, position_packet);
                        }
                    }
                }


                enet_packet_destroy(enet_event.packet);





                break;















            case ENET_EVENT_TYPE_DISCONNECT:
                std::cout << (char*)enet_event.peer->data << " disconnected." << std::endl;

                clients.erase(enet_event.peer);

                if (enet_event.peer->data) {
                    delete[](char*)enet_event.peer->data;
                    enet_event.peer->data = NULL;
                }
                break;
            }
        }
    }


    enet_host_destroy(server);
}
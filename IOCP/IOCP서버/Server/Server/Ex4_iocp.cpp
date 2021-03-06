#include"stdfx.h"
#include"Network.h"
using namespace std;



int main()
{
	Network* network = new Network();
	wcout.imbue(locale("korean"));
	WSADATA WSAData;
	int ret = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if(ret != 0) error_display("Init",WSAGetLastError());

	{
		Network* net = Network::GetInstance();

		net->g_s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);

		SOCKADDR_IN server_addr;
		ZeroMemory(&server_addr, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(SERVER_PORT);
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		bind(net->g_s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
		listen(net->g_s_socket, SOMAXCONN);

		net->g_h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(net->g_s_socket), net->g_h_iocp, 0, 0);

		net->start_accept();

		vector<thread> worker_threads;
		thread ai_thread{ &Network::do_timer,net };
		for (int i = 0; i < THREAD_NUM; ++i)
			worker_threads.emplace_back(&Network::worker, net);

		ai_thread.join();
		for (auto& th : worker_threads)
			th.join();
		delete network;
	}
	WSACleanup();
}
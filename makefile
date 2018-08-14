all:
	g++ -O2 -std=c++17 -pthread -o sikradio-sender sikradio_sender.cpp common/common.cpp sikradio_sender/audio_transmission.cpp sikradio_sender/sikradio_listening.cpp
	g++ -O2 -std=c++17 -pthread -o sikradio-receiver sikradio_receiver.cpp common/common.cpp sikradio_receiver/receive_managment.cpp sikradio_receiver/receive_mcast_listening.cpp
	
	
clean:
	rm -f sikradio-sender
	rm -f sikradio-receiver
	


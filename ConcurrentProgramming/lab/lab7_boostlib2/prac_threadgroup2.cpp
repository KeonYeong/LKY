#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <math.h>

#define NUM_THREAD_IN_POOL 4

boost::asio::io_service io_forPrint;

bool IsPrime(int n) {
	if (n < 2) {
		return false;
	}

	for (int i = 2; i <= sqrt(n); i++) {
		if (n % i == 0) {
			return false;
		}
	}
	return true;
}

void Print(int r_start, int r_end, int s_number, int result){
	std::cout << "(" << s_number << ")number of primes in " << r_start << " ~ " << r_end << " is " << result << std::endl;
}


void ThreadFunc(int r_start, int r_end, int s_number) {
	long cnt_prime = 0;
	for (int i = r_start; i <= r_end; i++) {
			if (IsPrime(i)) {
				cnt_prime++;
			}
		}
	io_forPrint.post(boost::bind(
				Print, r_start, r_end, s_number, cnt_prime));
}

int main(void) {
	int range_start;
	int range_end;
	int sequence_number = 0;
	boost::asio::io_service io;
	boost::thread_group threadpool;
	boost::thread_group threadpool_result;
	boost::asio::io_service::work* work = new boost::asio::io_service::work(io);

	boost::asio::io_service::work* work2 = new boost::asio::io_service::work(io_forPrint);
	for (int i = 0 ; i < NUM_THREAD_IN_POOL ; i++){
		threadpool.create_thread(boost::bind(
					&boost::asio::io_service::run, &io));
	}
	threadpool_result.create_thread(boost::bind(
				&boost::asio::io_service::run, &io_forPrint));

	while(1){
		std::cin >> range_start;
		if(range_start == -1) break;
		std::cin >> range_end;
		io.post(boost::bind(
					ThreadFunc, range_start, range_end, sequence_number));
		sequence_number++;
	}

	delete work;
	threadpool.join_all();
	delete work2;
	threadpool_result.join_all();
	io.stop();
	io_forPrint.stop();

	return 0;
}


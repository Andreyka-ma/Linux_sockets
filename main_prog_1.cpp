#include<iostream>
#include<future>
#include<mutex>
#include <condition_variable>
        
class Semaphore {
public:
    Semaphore (int count_ = 0)
    : count(count_) 
    {
    }
    
    inline void release() {
        std::unique_lock<std::mutex> lock(mtx);
        count++;
        //notify the waiting thread
        cv.notify_one();
    }
    inline void acquire() {
        std::unique_lock<std::mutex> lock(mtx);
        while(count == 0) {
            //wait on the mutex until notify is called
            cv.wait(lock);
        }
        count--;
    }
private:
    std::mutex mtx;
    std::condition_variable cv;
    int count;
};

class MTBuff {
public:
	MTBuff() : buff(""), exit_prog(0) {
		// ������ ������� ������ � ������
		// ����� 1
		std::future<void> fut1 = std::async(std::launch::async, &MTBuff::write_to_buff, this);
		// ����� 2
		std::future<void> fut2 = std::async(std::launch::async, &MTBuff::read_buff, this);
	}
	void write_to_buff() {
		std::string input;
		while (!exit_prog) {
			// ��������� ������, ���� �� �����. ������� 
			while (true) {
				std::cout << "Thread 1 waiting for input (64 numbers max)...\n";
				std::cin >> input;
				if (input == "exit") { exit_prog = 1; input.clear(); break; }
				// �������� ����� ������ (�� ������ 64 ��������)
				if (input.size() > 64) {
					std::cout << "Too many characters\n";
					continue;
				}
				// ���������, ��� ������ ������� ������ �� ����
				if (!is_numbers(input)) {
					std::cout << "Not all numbers (or empty?)\n";
					continue;
				}
				break;
			}			

			// ������ ����������� �� ��������, 
			// ������ ���������� �� ��������� ����� "KB"
			sort_and_replace_even(input);

			// ������ ���������� � ����� �����
			write_sema.acquire(); // ���������� ������
			buff = input; 
			read_sema.release(); // ���������� ������
		}
	}
	void read_buff() {
		std::string data;
		while (!exit_prog) {
			// ��������� ������ �� ������ ������, ������� ������
			read_sema.acquire();  // ���������� ������
			data = buff;
			buff.clear();		  
			write_sema.release(); // ���������� ������

			// ����� � ��������� ���������� ������
			std::cout << "Thread 2 received: " << data << '\n';
			int sum = sum_nums_from_str(data);
			std::cout << "Thread 2 calculated: " << sum << '\n';

			// �������� ����� � ��������� 2
			// 
			//
		}
	}

	// ����� �������� ����, ��� 
	// ������ ������� ������ �� ����
	bool is_numbers(const std::string& s) const {
		bool nums = true;
		for (char c : s) {
			if (!std::isdigit(c)) {
				nums = false;
				break;
			}
		}
		if (s.empty()) { std::cout << "Empty\n"; }
		return !s.empty() && nums;
	}

	// ����� ��������� ������ ��� ������ 1.
	// ��������� ������ �� ��������, ������� 
	// �������� � ������� ���������� �� "KB"
	void sort_and_replace_even(std::string& input) {
		// ���������� ��������� ������, �.�. 
		// ����������� �������� �������� 
		// ��������� ��������� �� "KB"
		size_t const max_el = 10;
		int char_cnt[max_el] = { 0 };

		// ���������� ���������� ���������
		for (char const & c : input) {
			char_cnt[c - '0']++;
		}
		
		// �������� � input ��������������� 
		// ������������������, ������� 
		// ������ �������� �� "KB"
		input.clear();
		std::string out_elem;
		for (size_t i = 0; i < max_el; i++) {
			if (i % 2 == 0) { out_elem = "KB"; }
			else { out_elem = char('0' + i); }
			for (int j = 0; j < char_cnt[i]; j++) {
				// ����. �� ��������, ��������
				// �������� � ������ ������
				input = out_elem + input;
			}
		}
	}
	
	// ����� ��� ������� ����� ���� ���������, 
	// ������� �������� ���������� ����������.
	// ��������� ��������� ���� ������ ������.
	int sum_nums_from_str(std::string const& data) const {
		int res = 0;
		for (char const& c : data) {
			if (std::isdigit(c)) {
				res += c - '0';
			}
		}
		return res;
	}
	void buff_state() const {
		while (!exit_prog) {
			//Sleep(10000);
			std::cout << "\nBuff: " << buff << '\n';
		}
	}
private:
	std::string buff;
	bool exit_prog;
	Semaphore read_sema{0};
	Semaphore write_sema{1};
};

int main() {
	setlocale(LC_ALL, "rus");
	MTBuff B;
	std::cout << "Main\n1. ��������������� ��������� � ������ ����� ������ ������?\n";
	std::cout << "2. ��������� �� main ����� �������? ������ ����� ������ ����� '������' �� MTBuff?\n";
	std::cout << "3. ���������� while(true)?\n";
	return 0;
}
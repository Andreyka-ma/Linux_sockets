#include<iostream>
#include<future>
#include<mutex>
#include <condition_variable>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

class Semaphore {
public:
    Semaphore (int count_)
    	: count(count_) {}
    
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
	MTBuff() : buff(""), exit_prog(0), connected(0) {
		// Запуск потоков чтения и записи
		// Поток 1
		std::future<void> fut1 = std::async(std::launch::async, &MTBuff::write_to_buff, this);
		// Поток 2
		std::future<void> fut2 = std::async(std::launch::async, &MTBuff::read_buff, this);
		
		// Создание соединения с программой 2
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		
		// localhost    
		struct hostent *server = gethostbyname("localhost");
		
		// Адрес сервера 
		int portno = 21947;
		struct sockaddr_in serv_addr;    
		bzero((char *) &serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(portno);
		bcopy((char *)server->h_addr, 
		     (char *)&serv_addr.sin_addr.s_addr,
		     server->h_length);
		     
		// Поддержание соединения с сервером
		while(!exit_prog) {

			if (!connected) {
				try_connect(sockfd, serv_addr);	
			}
			connect_sema.acquire();
		}
	}
	~MTBuff() { close(sockfd); }
	
	// Метод для соединения/переподключения к программе 2
	int try_connect(int sockfd, struct sockaddr_in serv_addr) {
		std::cout << "Connecting to Prog_2...\n";     
		while ((connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
				&& !exit_prog)  { usleep(1000000); }   
		std::cout << "Connected.\n";
		connected = 1;
		return 0;
	}
	
	void write_to_buff() {
		std::cout << "Prog 1. Type 'exit' to quit.\n";
		std::string input;
		while (!exit_prog) {
			// Считываем строку, пока не удовл. условия 
			while (true) {
				std::cout << "Thread 1 waiting for user input (64 numbers max)...\n";
				std::cin >> input;
				// Выход из программы
				if (input == "exit") { 
					exit_prog = 1;
					connect_sema.release(); 
					read_sema.release(); 
					return; 
				}
				// Проверка длины строки (не больше 64 символов)
				if (input.size() > 64) {
					std::cout << "Error - too many characters.\n";
					continue;
				}
				// Проверяем, что строка состоит только из цифр
				if (!is_numbers(input)) {
					std::cout << "Error - not all characters are numbers.\n";
					continue;
				}
				break;
			}			
			
			// Строка сортируется по убыванию, 
			// четные заменяются на латинские буквы "KB"
			sort_and_replace_even(input);

			// Строка помещается в общий буфер
			write_sema.acquire(); // Блокировка записи
			buff = input; 
			read_sema.release(); // Разрешение чтения
		}
	}
	void read_buff() {
		std::string data;
		while (!exit_prog) {
			// Получение данных из общего буфера, очистка буфера
			read_sema.acquire();  // Блокировка чтения
			data = buff;
			buff.clear();		  
			write_sema.release(); // Разрешение записи

			if (exit_prog) { return; }

			// Вывод и обработка полученных данных
			std::cout << "Thread 2 received: " << data << '\n';
			int sum = sum_nums_from_str(data);
			//std::cout << "Thread 2 calculated: " << sum << '\n';

			// Проверка соединения с программой 2
			bool lost_con = 1;
			if (connected){ read(sockfd, &lost_con, 1); }		
			if (connected && lost_con) { 
				std::cout << "Prog_2 connection lost, trying to reconnect...\n";
	 		    close(sockfd);
	 		    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	 		    connected = 0;
	 		    connect_sema.release();
			}
			if (connected) {
				// Передача суммы в программу 2
				std::cout << "Thread 2 sending data to Prog_2...\n";
				int sent_num = write(sockfd, &sum, sizeof(int));
			}       
		}
	}

	// Метод проверки того, что 
	// строка состоит только из цифр
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

	// Метод обработки строки для потока 1.
	// Сортирует строку по убыванию, заменяя 
	// элементы с четными значениями на "KB"
	void sort_and_replace_even(std::string& input) {
		// Сортировка подсчетом удобна, т.к. 
		// потребуется заменить значения 
		// некоторых элементов на "KB"
		size_t const max_el = 10;
		int char_cnt[max_el] = { 0 };

		// Запоминаем количество элементов
		for (char const & c : input) {
			char_cnt[c - '0']++;
		}
		
		// Помещаем в input отсортированную 
		// последовательность, заменяя 
		// четные значения на "KB"
		input.clear();
		std::string out_elem;
		for (size_t i = 0; i < max_el; i++) {
			if (i % 2 == 0) { out_elem = "KB"; }
			else { out_elem = char('0' + i); }
			for (int j = 0; j < char_cnt[i]; j++) {
				// Сорт. по убыванию, элементы
				// помещаем в начало строки
				input = out_elem + input;
			}
		}
	}
	
	// Метод для расчета суммы всех элементов, 
	// которые являются численными значениями.
	// Элементом считается один символ строки.
	int sum_nums_from_str(std::string const& data) const {
		int res = 0;
		for (char const& c : data) {
			if (std::isdigit(c)) {
				res += c - '0';
			}
		}
		return res;
	}
private:
	std::string buff;
	bool exit_prog;
	Semaphore read_sema{0};
	Semaphore write_sema{1};
	Semaphore connect_sema{0};
	int sockfd;
	bool connected;
};

int main() {
	setlocale(LC_ALL, "rus");
	MTBuff B;
	std::cout << "Main\n1. Неоднозначность поведения в случае ввода пустой строки?\n";
	std::cout << "2. Считается ли main своим потоком? Почему вывод только после 'выхода' из MTBuff?\n";
	std::cout << "3. Переделать while(true)?\n";
	return 0;
}

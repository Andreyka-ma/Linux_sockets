#include <iostream>
#include <future>
#include <mutex>
#include <semaphore>
#include <string.h> 
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

class MTBuff {
public:
	MTBuff(int portno);
	~MTBuff();

	// Метод для соединения/переподключения к программе 2	
	int try_accept(int sockfd);
	
	// Метод для потока записи в буфер
	void write_buff();
	
	// Метод для потока чтения из буфера
	void read_buff();

	// Метод проверки того, что строка
	// состоит только из цифр и не пуста
	bool is_numbers(const std::string& s) const;

	// Метод обработки строки для потока 1.
	// Сортирует строку по убыванию, заменяя 
	// элементы с четными значениями на "KB".
	// (Элементом считается один символ строки)
	void sort_and_replace_even(std::string& input);
	
	// Метод для расчета суммы элементов строки, 
	// которые являются численными значениями.
	// (Элементом считается один символ строки)
	int sum_nums_from_str(std::string const& data) const;
	
private:
	std::string buff;
	std::binary_semaphore read_sema{0};
	std::binary_semaphore write_sema{1};
	std::mutex mtx;
	int sockfd;
	struct sockaddr_in serv_addr;
	int newsockfd;
	bool connected;
};

int main() {
	int port_num = 21947;
	MTBuff B(port_num);
	return 0;
}

MTBuff::MTBuff(int portno) : buff(""), connected(0) {
	std::cout << "Prog_1\n";
	// Запуск потоков чтения и записи
	// Поток 1
	std::future<void> fut1 = std::async(std::launch::async, &MTBuff::write_buff, this);
	// Поток 2
	std::future<void> fut2 = std::async(std::launch::async, &MTBuff::read_buff, this);
	
	// Используется TCP
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	// Разрешение переиспользования порта
	const int enable = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
	
	// Привязка сокета к адресу
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	bind(sockfd, (struct sockaddr *) &serv_addr,
		 sizeof(serv_addr)); 
	
	listen(sockfd,5);
	
	// Подкл./поддерж. соединения:  периодически 
	// (каждые 0.5с) обмениваем данные с программой 2
	while(true) {
		// Исключаем возможность одновременного для 
		// данного потока и потока 2 выполнения кода
		mtx.lock();
		if (connected) {
			// Чтобы программа 2 не путала эти
			// данные с основными, можно отправлять
			// подставное значение (например -2)
			int val = -2; 
			write(newsockfd,&val,sizeof(int));
			
			bool lost_con = 1;
			read(newsockfd,&lost_con,1);
			if (lost_con) {
				// Потеряно соединение
				std::cout << "Prog_2 connection lost.\n";
				close(newsockfd);
				connected = 0;
			}
		}
		// После выяснения состояния
		// соединения отпускаем мьютекс
		mtx.unlock(); 
		if (!connected) {
			// Восстановление соединения
			newsockfd = try_accept(sockfd);	
		}
		sleep(0.5);
	}
}

MTBuff::~MTBuff() { close(newsockfd); close(sockfd); }

// Метод для соединения/переподключения к программе 2	
int MTBuff::try_accept(int sockfd) {
	std::cout << "Waiting for Prog_2 connection...\n";
	int retsockfd = accept(sockfd, NULL, NULL);
	std::cout << "Connected.\n";
	connected = 1;
	return retsockfd;
}

// Метод для потока записи в буфер (поток 1)
void MTBuff::write_buff() {
	std::string input;
	while (true) {
		// Считываем строку, пока не удовл. условия 
		while (true) {
			std::cout << "Thread 1 waiting for user input (64 numbers max)...\n";
			std::cin >> input;
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
		
		// Строка сортируется по убыванию, четные 
		// значения заменяются на латинские буквы "KB"
		sort_and_replace_even(input);

		// Строка помещается в общий буфер
		write_sema.acquire(); // Блокировка записи
		buff = input; 
		read_sema.release(); // Разрешение чтения
	}
}

// Метод для потока чтения из буфера (поток 2)
void MTBuff::read_buff() {
	std::string data;
	while (true) {
		// Получение данных из общего буфера, очистка буфера
		read_sema.acquire();  // Блокировка чтения
		data = buff;
		buff.clear();		  
		write_sema.release(); // Разрешение записи

		// Вывод и обработка полученных данных
		std::cout << "Thread 2 received: " << data << '\n';
		int sum = sum_nums_from_str(data);
		
		// Передача данных.
		// Исключаем возможность одновременного для 
		// нескольких потоков выполнения кода 
		mtx.lock(); 
		if (connected) {
			// Передача суммы программе 2
			std::cout << "Sending data to Prog_2.\n";
			write(newsockfd,&sum,sizeof(int));
			
			// Обратная связь с программой 2
			bool lost_con = 1;
			read(newsockfd,&lost_con,1);
			if (lost_con) {
				// Потеряно соединение
				std::cout << "Prog_2 connection lost.\n";
				close(newsockfd);
				connected = 0;
			}
		}
		// После завершения передачи
		// отпускаем мьютекс
		mtx.unlock();    
	}
}

// Метод проверки того, что строка
// состоит только из цифр и не пуста
bool MTBuff::is_numbers(const std::string& s) const {
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
// элементы с четными значениями на "KB".
// (Элементом считается один символ строки)
void MTBuff::sort_and_replace_even(std::string& input) {
	// Сортировка подсчетом удобна, т.к. 
	// потребуется заменить значения 
	// некоторых элементов на "KB"
	size_t const max_el = 10;
	int sym_cnt[max_el] = { 0 };

	// В массив sym_cnt записывается 
	// количество вхождения каждого элемента
	// в порядке 9, 8, 7, ..., 2, 1, 0.
	for (char const & c : input) {
		sym_cnt['9' - c]++;
	}
	
	// Помещаем в input отсортированную 
	// последовательность, заменяя 
	// четные значения на "KB"
	input.clear();
	std::string out_elem;
	for (size_t i = 0; i < max_el; i++) {
		// (i % 2 != 0) - четные значения по нечетным индексам
		if (i % 2 != 0) { out_elem = "KB"; } 
		else { out_elem = char('9' - i); }
		for (int j = 0; j < sym_cnt[i]; j++) {
			input += out_elem;
		}
	}
}

// Метод для расчета суммы элементов строки, 
// которые являются численными значениями.
// (Элементом считается один символ строки)
int MTBuff::sum_nums_from_str(std::string const& data) const {
	int res = 0;
	for (char const& c : data) {
		if (std::isdigit(c)) {
			res += c - '0';
		}
	}
	return res;
}

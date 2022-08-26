#include<iostream>

bool check(int n) {
	return (n > 9) && (n % 32 == 0);
}

int main() {
	int n;
	// Получение значения из программы 1
	std::cout << "Waiting for input...\n";
	std::cin >> n;
	// Выводим сообщение о полученных данных 
	// если полученное значение состоит из 
	// более 2-ух символов и кратно 32  
	if (check(n)) {
		std::cout << "Received data: " << n << '\n';
	}
	else {
		std::cout << "Error - incorrect data\n";
	}

	main();
	setlocale(LC_ALL, "rus");
	return 0;
}
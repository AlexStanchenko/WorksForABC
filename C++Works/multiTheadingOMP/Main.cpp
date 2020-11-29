#include<iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <sstream>
#include <omp.h>


using namespace std;
// Количество множеств на входе
static int countOfSets = 4;
// Лист с результатом выполнения программы
static list<string> resultSets;

/// <summary>
/// Класс точки, являющейся декартовым произведением 2 множеств
/// </summary>
class Point
{
public:

	/// <summary>
	/// Создает экземпляр класса
	/// </summary>
	Point()
	{
		x_ = 0;
		y_ = 0;
	}

	/// <summary>
	/// Создает экземпляр класса
	/// </summary>
	/// <param name="x">элемент из первого множества</param>
	/// <param name="y">элемент из второго множетсва</param>
	Point(int x, int y)
	{
		x_ = x;
		y_ = y;
	}

	/// <summary>
	/// Функция преобразования экземпляра класса в строку
	/// </summary>
	/// <param name="out">Поток</param>
	/// <param name="point">Экземпляр класса</param>
	/// <returns>строковое представление класса</returns>
	friend	ostream& operator<< (ostream& out, const Point& point)
	{
		out << "(" << point.x_ << "х" << point.y_ << ")";
		return out;
	}

private:
	// Поля- элементы множества
	int x_, y_;
};


/// <summary>
/// Записывает информацию в файл
/// </summary>
/// <param name="path">Путь к файлу для записи</param>
void WriteFile(string path)
{
	ofstream fout;
	fout.open(path, ios_base::trunc);
	copy(resultSets.begin(), resultSets.end(), ostream_iterator<string>(fout, ""));
	fout.close();
}


/// <summary>
/// Метод по считыванию информации из файла
/// </summary>
/// <param name="path">Путь к файлу</param>
/// <returns>Вектор множеств чисел</returns>
vector<vector<int>> ReadFile(string path)
{
	vector<vector<int>> SetsOfNumbers;
	// Поток к файлу
	ifstream in(path);
	if (in.is_open())
	{
		// Количество множеств
		for (int i = 0; i < countOfSets; i++)
		{
			vector <int> numbers;
			// Мощность множества
			int size;
			in >> size;


			if (size <= 0)
			{
				throw exception("Мощность множества не может быть меньше единицы!");
			}

			// Считываем множество
			for (int i = 0; i < size; i++)
			{
				int x;
				in >> x;
				numbers.push_back(x);
			}

			SetsOfNumbers.push_back(numbers);
		}
	}
	in.close();

	return SetsOfNumbers;
}


int countOperation(int countSet) {

	int count = 1;


	for (int i = 2; i < countSet; i++)
	{
		count *= i;
	}

	return count;
}


/// <summary>
/// Выполняет прямое произведение множеств на входе и записывает результат в файл
/// </summary>
/// <param name="id">Номер потока</param>
/// <param name="i">Номер первого множества</param>
/// <param name="j">Номер второго множества</param>
/// <param name="firstSet">Первое множество</param>
/// <param name="secondSet">Второе множество</param>
void threadFunction(string nameSet, vector<int> firstSet, vector<int> secondSet) {


	list<Point> resultSet;
	// Заполняем лист, являющийся множеством декартовых произведений элементов двух множеств на входе
	for (int i = 0; i < firstSet.size(); i++)
	{
		for (int j = 0; j < secondSet.size(); j++)
		{
			resultSet.push_back(Point(firstSet[i], secondSet[j]));
		}
	}


	// Создаем поток, по записи информации в строку
	// Создаем результирующую строчку
	ostringstream strout;
	// Запись индекса потока и названия множества в строку
	strout << "Thread-" << omp_get_thread_num() << " " << nameSet << " { ";
	// Запись множества
	copy(resultSet.begin(), resultSet.end(), ostream_iterator<Point>(strout, " "));
	strout << "}" << endl;
	string resultString = strout.str();

	// Добавляем строку для ввывода в контейнер результата
	resultSets.push_back(resultString);
}



int main(int argc, char* argv[]) {
	// Установка локализации
	setlocale(LC_ALL, "Russian");

	int countThreads = stoi(argv[1]);

	try {

		if (countThreads < 1 || countThreads > 6) {
			throw exception("Командная строка имела не правильный формат. Количество потоков может быть от 1 до 6!");
		}

		// Устанавливаем количестов потоков для OMP
		omp_set_num_threads(countThreads);
		vector<vector<int>> setsOfNumbers;

		// Считывание множеств из файла
		setsOfNumbers = ReadFile(argv[2]);

		vector<vector<vector<int>>> operations;
		vector<string> nameOperation;


		// Создаем будущие задачи для потоков
		for (int i = 0; i < countOfSets - 1; i++)
		{
			for (int j = i + 1; j < countOfSets; j++)
			{
				vector<vector<int>> operation;
				operation.push_back(setsOfNumbers[i]);
				operation.push_back(setsOfNumbers[j]);
				operations.push_back(operation);
				nameOperation.push_back("A" + to_string(i) + "A" + to_string(j));
			}
		}



		// Выполняем задачи в паралельной области
		#pragma omp parallel
		{
			// Выполняем задачи фор в параллельном режиме
			#pragma omp for 
			for (int i = 0; i < countOperation(countOfSets); i++)
			{
				threadFunction(nameOperation[i], operations[i][0], operations[i][1]);
			}

		}


		// запись в файл
		WriteFile(argv[3]);
	}
	catch (exception ex)
	{
		cout << ex.what() << endl;
		cout << "Пример командной строки:{countTheards} {pathToInputFile} {pathToOutputFile}" << endl;
	}
	return 0;
}

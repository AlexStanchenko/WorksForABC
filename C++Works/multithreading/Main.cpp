#include<iostream>
#include <fstream>
#include <string>
#include<thread>
#include <vector>
#include <list>
#include <mutex>
#include <sstream>


using namespace std;

condition_variable cond;
mutex g_lock;
static int countOfSets = 4;
static list<int> freeThreads;
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


/// <summary>
/// Выполняет прямое произведение множеств на входе и записывает результат в файл
/// </summary>
/// <param name="id">Номер потока</param>
/// <param name="i">Номер первого множества</param>
/// <param name="j">Номер второго множества</param>
/// <param name="firstSet">Первое множество</param>
/// <param name="secondSet">Второе множество</param>
void threadFunction(int id, int i, int j, vector<int> firstSet, vector<int> secondSet) {


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
	// Создае результирующую строчку
	ostringstream strout;
	strout << "Thread-" << id << " A" << i + 1 << "A" << j + 1 << " { ";
	copy(resultSet.begin(), resultSet.end(), ostream_iterator<Point>(strout, " "));
	strout << "}" << endl;
	string resultString = strout.str();


	// Блокируем все потоки
	// то есть заходим в критическую секцию
	g_lock.lock();
	// Добавляем строку для ввывода в контейнер результата
	resultSets.push_back(resultString);
	// Кладем индекс потока в лист с индексами освободившихся потоков
	freeThreads.push_back(id);
	// Сигнализируем об окончании работы данного потока
	cond.notify_all();
	// Разблокируем остальные потоки
	g_lock.unlock();
}



int main(int argc, char* argv[]) {
	// Установка локализации
	setlocale(LC_ALL, "Russian");

	int countThreads = stoi(argv[1]);


	try {

		if (countThreads < 1 || countThreads > 6) {
			throw exception("Количество потоков может быть от 1 до 6");
		}


		int countOperation = 0;
		thread* myThreads = new thread[countThreads];
		vector<vector<int>> setsOfNumbers;

		// Считывание множеств из файла
		setsOfNumbers = ReadFile(argv[2]);


		// Создание потоков и соответствующих задач для них
		for (int i = 0; i < countOfSets - 1; i++)
		{

			for (int j = i + 1; j < countOfSets; j++)
			{
				// Первоначальное присвоение операций потокам 
				if (countOperation != countThreads)
				{
					myThreads[countOperation] = thread(threadFunction, countOperation, i, j, setsOfNumbers[i], setsOfNumbers[j]);
					countOperation++;
				}
				// Если операций больше чем потоков, то они раздаются в следующем соответствии
				// Первый закончил, первый получил новую задачу
				else
				{
					// Проверка есть ли свободные потоки
					// Если нет, то ждем первый освободившиеся поток
					if (freeThreads.empty()) {
						unique_lock<mutex> lk(g_lock);
						cond.wait(lk, [&] { return !freeThreads.empty(); });
					}
					// Получаем индекс свободного потока
					int countss = freeThreads.back();
					// Выкидываем индекс потока из контейнера свободных потоков
					freeThreads.pop_back();
					// Присоединяем свободный поток
					myThreads[countss].join();
					// Назнчаем новую задачу
					myThreads[countss] = thread(threadFunction, countss, i, j, setsOfNumbers[i], setsOfNumbers[j]);
				}

			}
		}

		// Выполняем присоединение всех дополнительных потоков
		for (int i = 0; i < countThreads; i++)
		{
			myThreads[i].join();
		}


		// запись в файл
		WriteFile(argv[3]);


	}
	catch (exception ex)
	{
		cout << ex.what() << endl;
	}
	return 0;
}

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
/// ����� �����, ���������� ���������� ������������� 2 ��������
/// </summary>
class Point
{
public:

	/// <summary>
	/// ������� ��������� ������
	/// </summary>
	Point()
	{
		x_ = 0;
		y_ = 0;
	}

	/// <summary>
	/// ������� ��������� ������
	/// </summary>
	/// <param name="x">������� �� ������� ���������</param>
	/// <param name="y">������� �� ������� ���������</param>
	Point(int x, int y)
	{
		x_ = x;
		y_ = y;
	}

	/// <summary>
	/// ������� �������������� ���������� ������ � ������
	/// </summary>
	/// <param name="out">�����</param>
	/// <param name="point">��������� ������</param>
	/// <returns>��������� ������������� ������</returns>
	friend	ostream& operator<< (ostream& out, const Point& point)
	{
		out << "(" << point.x_ << "�" << point.y_ << ")";
		return out;
	}

private:
	// ����- �������� ���������
	int x_, y_;
};


/// <summary>
/// ���������� ���������� � ����
/// </summary>
/// <param name="path">���� � ����� ��� ������</param>
void WriteFile(string path)
{
	ofstream fout;
	fout.open(path, ios_base::trunc);
	copy(resultSets.begin(), resultSets.end(), ostream_iterator<string>(fout, ""));
	fout.close();
}


/// <summary>
/// ����� �� ���������� ���������� �� �����
/// </summary>
/// <param name="path">���� � �����</param>
/// <returns>������ �������� �����</returns>
vector<vector<int>> ReadFile(string path)
{
	vector<vector<int>> SetsOfNumbers;
	// ����� � �����
	ifstream in(path);
	if (in.is_open())
	{
		// ���������� ��������
		for (int i = 0; i < countOfSets; i++)
		{
			vector <int> numbers;
			// �������� ���������
			int size;
			in >> size;


			if (size <= 0)
			{
				throw exception("�������� ��������� �� ����� ���� ������ �������!");
			}

			// ��������� ���������
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
/// ��������� ������ ������������ �������� �� ����� � ���������� ��������� � ����
/// </summary>
/// <param name="id">����� ������</param>
/// <param name="i">����� ������� ���������</param>
/// <param name="j">����� ������� ���������</param>
/// <param name="firstSet">������ ���������</param>
/// <param name="secondSet">������ ���������</param>
void threadFunction(int id, int i, int j, vector<int> firstSet, vector<int> secondSet) {


	list<Point> resultSet;
	// ��������� ����, ���������� ���������� ���������� ������������ ��������� ���� �������� �� �����
	for (int i = 0; i < firstSet.size(); i++)
	{
		for (int j = 0; j < secondSet.size(); j++)
		{
			resultSet.push_back(Point(firstSet[i], secondSet[j]));
		}
	}




	// ������� �����, �� ������ ���������� � ������
	// ������ �������������� �������
	ostringstream strout;
	strout << "Thread-" << id << " A" << i + 1 << "A" << j + 1 << " { ";
	copy(resultSet.begin(), resultSet.end(), ostream_iterator<Point>(strout, " "));
	strout << "}" << endl;
	string resultString = strout.str();


	// ��������� ��� ������
	// �� ���� ������� � ����������� ������
	g_lock.lock();
	// ��������� ������ ��� ������� � ��������� ����������
	resultSets.push_back(resultString);
	// ������ ������ ������ � ���� � ��������� �������������� �������
	freeThreads.push_back(id);
	// ������������� �� ��������� ������ ������� ������
	cond.notify_all();
	// ������������ ��������� ������
	g_lock.unlock();
}



int main(int argc, char* argv[]) {
	// ��������� �����������
	setlocale(LC_ALL, "Russian");

	int countThreads = stoi(argv[1]);


	try {

		if (countThreads < 1 || countThreads > 6) {
			throw exception("���������� ������� ����� ���� �� 1 �� 6");
		}


		int countOperation = 0;
		thread* myThreads = new thread[countThreads];
		vector<vector<int>> setsOfNumbers;

		// ���������� �������� �� �����
		setsOfNumbers = ReadFile(argv[2]);


		// �������� ������� � ��������������� ����� ��� ���
		for (int i = 0; i < countOfSets - 1; i++)
		{

			for (int j = i + 1; j < countOfSets; j++)
			{
				// �������������� ���������� �������� ������� 
				if (countOperation != countThreads)
				{
					myThreads[countOperation] = thread(threadFunction, countOperation, i, j, setsOfNumbers[i], setsOfNumbers[j]);
					countOperation++;
				}
				// ���� �������� ������ ��� �������, �� ��� ��������� � ��������� ������������
				// ������ ��������, ������ ������� ����� ������
				else
				{
					// �������� ���� �� ��������� ������
					// ���� ���, �� ���� ������ �������������� �����
					if (freeThreads.empty()) {
						unique_lock<mutex> lk(g_lock);
						cond.wait(lk, [&] { return !freeThreads.empty(); });
					}
					// �������� ������ ���������� ������
					int countss = freeThreads.back();
					// ���������� ������ ������ �� ���������� ��������� �������
					freeThreads.pop_back();
					// ������������ ��������� �����
					myThreads[countss].join();
					// �������� ����� ������
					myThreads[countss] = thread(threadFunction, countss, i, j, setsOfNumbers[i], setsOfNumbers[j]);
				}

			}
		}

		// ��������� ������������� ���� �������������� �������
		for (int i = 0; i < countThreads; i++)
		{
			myThreads[i].join();
		}


		// ������ � ����
		WriteFile(argv[3]);


	}
	catch (exception ex)
	{
		cout << ex.what() << endl;
	}
	return 0;
}

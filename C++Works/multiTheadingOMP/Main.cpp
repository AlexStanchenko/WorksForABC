#include<iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <sstream>
#include <omp.h>


using namespace std;
// ���������� �������� �� �����
static int countOfSets = 4;
// ���� � ����������� ���������� ���������
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


int countOperation(int countSet) {

	int count = 1;


	for (int i = 2; i < countSet; i++)
	{
		count *= i;
	}

	return count;
}


/// <summary>
/// ��������� ������ ������������ �������� �� ����� � ���������� ��������� � ����
/// </summary>
/// <param name="id">����� ������</param>
/// <param name="i">����� ������� ���������</param>
/// <param name="j">����� ������� ���������</param>
/// <param name="firstSet">������ ���������</param>
/// <param name="secondSet">������ ���������</param>
void threadFunction(string nameSet, vector<int> firstSet, vector<int> secondSet) {


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
	// ������� �������������� �������
	ostringstream strout;
	// ������ ������� ������ � �������� ��������� � ������
	strout << "Thread-" << omp_get_thread_num() << " " << nameSet << " { ";
	// ������ ���������
	copy(resultSet.begin(), resultSet.end(), ostream_iterator<Point>(strout, " "));
	strout << "}" << endl;
	string resultString = strout.str();

	// ��������� ������ ��� ������� � ��������� ����������
	resultSets.push_back(resultString);
}



int main(int argc, char* argv[]) {
	// ��������� �����������
	setlocale(LC_ALL, "Russian");

	int countThreads = stoi(argv[1]);

	try {

		if (countThreads < 1 || countThreads > 6) {
			throw exception("��������� ������ ����� �� ���������� ������. ���������� ������� ����� ���� �� 1 �� 6!");
		}

		// ������������� ���������� ������� ��� OMP
		omp_set_num_threads(countThreads);
		vector<vector<int>> setsOfNumbers;

		// ���������� �������� �� �����
		setsOfNumbers = ReadFile(argv[2]);

		vector<vector<vector<int>>> operations;
		vector<string> nameOperation;


		// ������� ������� ������ ��� �������
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



		// ��������� ������ � ����������� �������
		#pragma omp parallel
		{
			// ��������� ������ ��� � ������������ ������
			#pragma omp for 
			for (int i = 0; i < countOperation(countOfSets); i++)
			{
				threadFunction(nameOperation[i], operations[i][0], operations[i][1]);
			}

		}


		// ������ � ����
		WriteFile(argv[3]);
	}
	catch (exception ex)
	{
		cout << ex.what() << endl;
		cout << "������ ��������� ������:{countTheards} {pathToInputFile} {pathToOutputFile}" << endl;
	}
	return 0;
}

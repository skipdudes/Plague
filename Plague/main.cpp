#include <iostream>
#include <string>
#include <list>
#include <queue>
#include <map>
#include <vector>
#include <algorithm>
using namespace std;

#define SOURCE_NAME "s"
#define SINK_NAME "t"
#define DAY_NAME "d"
#define TIME_NAME "t"
#define DEPARTMENT_NAME "k"
#define RESEARCHER_NAME "r"
#define GROUP_NAME "g"
#define FF_NO_PRECURSOR_NAME "FF_NONE"

struct Vertex;
struct Edge;
struct Researcher;
struct Department;
struct Task;
struct Group;
struct Timetable;

struct Vertex
{
	string data;
	list<Edge*> edges;
};

struct Edge
{
	Vertex* connection = NULL;
	Edge* symetric = NULL;
	int capacity = 0;
	int flow = 0;
	bool residual = 0;
};

struct Researcher
{
	string name;
	string realName;
	int points = 0;
	int hours = 0;
	int hoursPerDay = 0;
	Department* department = NULL;
};

struct Department
{
	string name;
	string realName;
	int researcherNumber = 0;
	Researcher* researchers = NULL;
};

struct Task
{
	string name;
	int day = 0;
	int time = 0;
	int minHours = 0;
	int maxHours = 0;
	Group* group = NULL;
};

struct Group
{
	string name;
	string realName;
	int taskNumber = 0;
	int hours = 0;
	int minimalHoursOfTasks = 0;
	Task* tasks = NULL;
};

struct Timetable
{
	string name;
	int points = 0;
	list<Researcher*> researchers;
	list<Department*> departments;
	map<string, int> departmentResearcherNumbers;
	map<string, int> researcherHours;
	map<string, queue<Task*>> researcherTasks;
};

//znajduje odpowiedni dzien lub slot czasu
bool findInList(list<int> list, int n)
{
	for (auto i : list)
		if (i == n)
			return true;

	return false;
}

//znajduje wierzcholek o okreslonej nazwie
Vertex* findVertex(list<Vertex*> List, const string& data)
{
	for (auto i : List)
		if (i->data == data)
			return i;

	return NULL;
}

//tworzy nowy wierzcholek
Vertex* newVertex(const string& data)
{
	Vertex* tmp = new Vertex;
	tmp->data = data;
	return tmp;
}

//tworzy nowa krawedz do wierzcholka o nazwie "connection"
Edge* newEdge(const list<Vertex*>& List, const string& connection, int capacity, int flow)
{
	Edge* tmp = new Edge;

	Vertex* find = findVertex(List, connection);
	tmp->connection = find;

	tmp->symetric = NULL;
	tmp->capacity = capacity;
	tmp->flow = flow;
	tmp->residual = 0;
	return tmp;
}

//tworzy nowa krawedz i podpina ja do wierzcholka "source"
void connectVertices(list<Vertex*>& List, const string& source, const string& connection, int capacity, int flow)
{
	Vertex* tmp = findVertex(List, source);
	if (tmp)
		tmp->edges.push_back(newEdge(List, connection, capacity, flow));
}

//wczytanie katedr i pracownikow
void loadDepartments(int& departmentNumber, Department*& departments)
{
	cin >> departmentNumber;
	departments = new Department[departmentNumber];

	for (int i = 0; i < departmentNumber; i++)
	{
		cin >> departments[i].realName;
		departments[i].name = departments[i].realName + DEPARTMENT_NAME;
		cin >> departments[i].researcherNumber;
		departments[i].researchers = new Researcher[departments[i].researcherNumber];

		for (int j = 0; j < departments[i].researcherNumber; j++)
		{
			cin >> departments[i].researchers[j].realName;
			departments[i].researchers[j].name = departments[i].name + departments[i].researchers[j].realName + RESEARCHER_NAME;
			cin >> departments[i].researchers[j].points;
			cin >> departments[i].researchers[j].hours;
			cin >> departments[i].researchers[j].hoursPerDay;
			departments[i].researchers[j].department = &departments[i];
		}
	}
}

//wczytanie grup zadan i zadan
void loadGroups(int& groupNumber, Group*& groups, int& totalHours, list<int>& days, list<int>& times)
{
	cin >> groupNumber;
	groups = new Group[groupNumber];

	for (int i = 0; i < groupNumber; i++)
	{
		cin >> groups[i].realName;
		groups[i].name = groups[i].realName + GROUP_NAME;
		cin >> groups[i].taskNumber >> groups[i].hours;

		totalHours += groups[i].hours;
		groups[i].minimalHoursOfTasks = 0;

		groups[i].tasks = new Task[groups[i].taskNumber];

		for (int j = 0; j < groups[i].taskNumber; j++)
		{
			//nadanie nazwy zadaniu rownej nazwie grupy + indeks zadania + 1
			groups[i].tasks[j].name = groups[i].name;
			char index = char(int((j + 1) + '0'));
			groups[i].tasks[j].name += index;

			//sprawdzenie listy dni i ewentualne dodanie nowego dnia do listy
			cin >> groups[i].tasks[j].day;
			if (!findInList(days, groups[i].tasks[j].day))
				days.push_back(groups[i].tasks[j].day);

			//sprawdzenie listy slotow czasu i ewentualne dodanie nowego slotu czasu do listy
			cin >> groups[i].tasks[j].time;
			if (!findInList(times, groups[i].tasks[j].time))
				times.push_back(groups[i].tasks[j].time);

			cin >> groups[i].tasks[j].minHours;
			groups[i].minimalHoursOfTasks += groups[i].tasks[j].minHours;
			cin >> groups[i].tasks[j].maxHours;
			groups[i].tasks[j].group = &groups[i];
		}
	}
}

//utworzenie wierzcholkow zrodla, zadan, grup i ujscia
void initList(list<Vertex*>& List, int groupNumber, Group* groups)
{
	List.push_back(newVertex(SOURCE_NAME));
	List.push_back(newVertex(SINK_NAME));

	for (int i = 0; i < groupNumber; i++)
	{
		List.push_back(newVertex(groups[i].name));
		connectVertices(List, groups[i].name, SINK_NAME, (groups[i].hours - groups[i].minimalHoursOfTasks), 0);

		for (int j = 0; j < groups[i].taskNumber; j++)
		{
			List.push_back(newVertex(groups[i].tasks[j].name));
			connectVertices(List, groups[i].tasks[j].name, SINK_NAME, groups[i].tasks[j].minHours, 0);
			connectVertices(List, groups[i].tasks[j].name, groups[i].name, (groups[i].tasks[j].maxHours - groups[i].tasks[j].minHours), 0);
		}
	}
}

//zwraca wartosc elementu listy znajdujacego sie w okreslonym miejscu
int returnListValue(list<int> list, int n)
{
	int value;
	int i = 0;
	value = list.front();
	list.pop_front();
	while (i < n)
	{
		value = list.front();
		list.pop_front();
		i++;
	}
	return value;
}

//dodaje pracownika do sieci
void addResearcher(list<Vertex*>& List, Researcher researcher, list<int> days, list<int> times, int groupNumber, Group* groups)
{
	List.push_back(newVertex(researcher.name));
	for (int i = 0; i < int(days.size()); i++)
	{
		int certainDay = returnListValue(days, i);
		string tmp = researcher.name + DAY_NAME;
		char index = char(int(certainDay + '0'));
		tmp += index;
		List.push_back(newVertex(tmp));

		for (int j = 0; j < int(times.size()); j++)
		{
			int certainTime = returnListValue(times, j);
			string tmp2 = tmp + TIME_NAME;
			char index2 = char(int(certainTime + '0'));
			tmp2 += index2;
			List.push_back(newVertex(tmp2));

			for (int k = 0; k < groupNumber; k++)
			{
				for (int l = 0; l < groups[k].taskNumber; l++)
				{
					if (groups[k].tasks[l].day == certainDay && groups[k].tasks[l].time == certainTime)
					{
						connectVertices(List, tmp2, groups[k].tasks[l].name, 1, 0);
					}
				}
			}
			connectVertices(List, tmp, tmp2, 1, 0);
		}
		connectVertices(List, researcher.name, tmp, researcher.hoursPerDay, 0);
	}
	connectVertices(List, SOURCE_NAME, researcher.name, researcher.hours, 0);
}

//tworzy krawedzi symetryczne
void createSymetricEdges(list<Vertex*>& List)
{
	for (auto i : List)
	{
		for (auto j : i->edges)
		{
			if (!(j->symetric))
			{
				Edge* tmp = new Edge;

				tmp->connection = i;
				tmp->symetric = j;
				tmp->capacity = 0;
				tmp->flow = 0;
				tmp->residual = 1;

				Vertex* find = findVertex(List, j->connection->data);
				if (find)
					find->edges.push_back(tmp);

				j->symetric = tmp;
			}
		}
	}
}

//usuwa wszystkie krawedzie wierzcholka
void deleteEdgesFromVertex(list<Vertex*>& List, const string& name)
{
	Vertex* find = findVertex(List, name);
	list<Edge*>::iterator k = find->edges.begin();
	while (k != find->edges.end())
	{
		find->edges.erase(k++);
	}
}

//usuwa okreslona krawedz od source do connection
void deleteSpecificEdge(list<Vertex*>& List, const string& source, const string& connection)
{
	Vertex* find = findVertex(List, source);
	list<Edge*>::iterator k = find->edges.begin();
	while (k != find->edges.end())
	{
		if ((*k)->connection->data == connection)
			find->edges.erase(k++);
		else
			++k;
	}
}

//usuwa wierzcholek o okreslonej nazwie
void deleteSpecificVertex(list<Vertex*>& List, const string& name)
{
	Vertex* find = findVertex(List, name);
	List.remove(find);
	delete find;
}

//usuwa pracownika z sieci
void deleteResearcher(list<Vertex*>& List, Researcher researcher, list<int> days, list<int> times, int groupNumber, Group* groups)
{
	for (int i = 0; i < int(days.size()); i++)
	{
		int certainDay = returnListValue(days, i);
		string tmp = researcher.name + DAY_NAME;
		char index = char(int(certainDay + '0'));
		tmp += index;

		for (int j = 0; j < int(times.size()); j++)
		{
			int certainTime = returnListValue(times, j);
			string tmp2 = tmp + TIME_NAME;
			char index2 = char(int(certainTime + '0'));
			tmp2 += index2;

			for (int k = 0; k < groupNumber; k++)
			{
				for (int l = 0; l < groups[k].taskNumber; l++)
				{
					if (groups[k].tasks[l].day == certainDay && groups[k].tasks[l].time == certainTime)
					{
						deleteSpecificEdge(List, groups[k].tasks[l].name, tmp2);
					}
				}
			}
			connectVertices(List, tmp, tmp2, 1, 0);

			deleteEdgesFromVertex(List, tmp2);
		}
		deleteEdgesFromVertex(List, tmp);
	}
	deleteEdgesFromVertex(List, researcher.name);
	deleteSpecificEdge(List, SOURCE_NAME, researcher.name);

	for (int i = 0; i < int(days.size()); i++)
	{
		int certainDay = returnListValue(days, i);
		string tmp = researcher.name + DAY_NAME;
		char index = char(int(certainDay + '0'));
		tmp += index;

		for (int j = 0; j < int(times.size()); j++)
		{
			int certainTime = returnListValue(times, j);
			string tmp2 = tmp + TIME_NAME;
			char index2 = char(int(certainTime + '0'));
			tmp2 += index2;

			deleteSpecificVertex(List, tmp2);
		}
		deleteSpecificVertex(List, tmp);
	}
	deleteSpecificVertex(List, researcher.name);
}

//usuwa wszystkich pracownikow z sieci
void deleteAllResearchers(list<Vertex*>& List, const list<int>& days, const list<int>& times, int groupNumber, Group* groups, int departmentNumber, Department* departments)
{
	for (int i = 0; i < departmentNumber; i++)
	{
		for (int j = 0; j < departments[i].researcherNumber; j++)
		{
			Vertex* find = findVertex(List, departments[i].researchers[j].name);
			if (find)
				deleteResearcher(List, departments[i].researchers[j], days, times, groupNumber, groups);
		}
	}
}

//znajduje zadanie o danej nazwie
Task* findTask(Group* groups, int groupNumber, const string& taskName)
{
	for (int i = 0; i < groupNumber; i++)
	{
		for (int j = 0; j < groups[i].taskNumber; j++)
		{
			if (groups[i].tasks[j].name == taskName)
				return &groups[i].tasks[j];
		}
	}
	return NULL;
}

//tworzy backup oryginalnych przeplywow i pojemnosci kanalow w sieci
void backupFlowAndCapacity(list<Vertex*>& List, map<string, int>& flowBackup, map<string, int>& capacityBackup)
{
	for (auto i : List)
	{
		for (auto j : i->edges)
		{
			string edgeId = i->data + "->" + j->connection->data;
			flowBackup[edgeId] = j->flow;
			capacityBackup[edgeId] = j->capacity;
		}
	}
}

//przywraca na podstawie backupu oryginalne przeplywy i pojemnosci kanalow w sieci
void restoreFlowAndCapacity(list<Vertex*>& List, map<string, int>& flowBackup, map<string, int>& capacityBackup)
{
	for (auto i : List)
	{
		for (auto j : i->edges)
		{
			string edgeId = i->data + "->" + j->connection->data;
			j->flow = flowBackup[edgeId];
			j->capacity = capacityBackup[edgeId];
		}
	}
}

//algorytm Forda-Fulkersona zwracajacy maksymalny przeplyw sieci
int fordFulkerson(list<Vertex*> List, Timetable& tmpTimetable, int groupNumber, Group* groups)
{
	map<string, int> flowBackup;
	map<string, int> capacityBackup;
	backupFlowAndCapacity(List, flowBackup, capacityBackup);

	int maxFlow = 0; //przeplyw sieci
	map<string, string> Precursors; //mapa poprzednikow konkretnych wierzcholkow wyszukiwanych przez BFS
	map<string, int> ResidualCapacity; //mapa przepustowosci rezydualnych siezek konczacych sie w krawedzi sieci
	queue<Vertex*> Queue; //kolejka przechowujaca wierzcholki dla BFS
	bool test = false; //pomocnicza zmienna

	while (true)
	{
		for (auto i : List)
			Precursors[i->data] = FF_NO_PRECURSOR_NAME;

		ResidualCapacity[SOURCE_NAME] = INT_MAX; //przepustowosc zrodla jest nieskonczona

		while (!Queue.empty())
			Queue.pop();

		Vertex* find = findVertex(List, SOURCE_NAME);
		Queue.push(find);

		while (!Queue.empty()) //szukanie sciezki rozszerzajacej w sieci
		{
			test = false;
			find = Queue.front();
			Queue.pop();

			for (auto i : find->edges) //dla wszystkich sasiadow wierzcholka kolejki
			{
				int residualCapacity = i->capacity - i->flow;

				if ((residualCapacity > 0) && (Precursors[i->connection->data] == FF_NO_PRECURSOR_NAME))
				{
					Precursors[i->connection->data] = find->data; //zapamietanie poprzednika
					ResidualCapacity[i->connection->data] = residualCapacity < ResidualCapacity[find->data] ? residualCapacity : ResidualCapacity[find->data];

					if (i->connection->data == SINK_NAME) //jesli dociera do ujscia
					{
						test = true; //znaleziono sciezke
						break;
					}
					else
					{
						Vertex* find2 = findVertex(List, i->connection->data);
						Queue.push(find2); //szukaj dalej
					}
				}
			}
			if (test) break;
		}
		if (!test) break; //wszystkie sciezki przeszukane

		maxFlow += ResidualCapacity[SINK_NAME];

		Vertex* find3 = findVertex(List, SINK_NAME);
		Vertex* source = findVertex(List, SOURCE_NAME);
		for (find3; find3 != source; find3 = find) //wroc po sciezce od ujscia do zrodla
		{
			find = findVertex(List, Precursors[find3->data]);

			for (auto i : find->edges)
			{
				if (i->connection->data == find3->data)
				{
					i->flow += ResidualCapacity[SINK_NAME]; //zwieksz przeplyw w kierunku zgodnym ze sciezka
					break;
				}
			}

			for (auto i : find3->edges)
			{
				if (i->connection->data == find->data)
				{
					i->flow -= ResidualCapacity[SINK_NAME]; //zmniejsz przeplyw w kierunku przeciwnym do sciezki
					break;
				}
			}
		}
	}

	Vertex* source = findVertex(List, SOURCE_NAME);
	for (auto i : source->edges)
	{
		tmpTimetable.researcherHours[i->connection->data] = i->flow; //ile godzin pracuje pracownik

		Vertex* researcher = findVertex(List, i->connection->data);
		for (auto j : researcher->edges)
		{
			Vertex* day = findVertex(List, j->connection->data);
			for (auto k : day->edges)
			{
				Vertex* time = findVertex(List, k->connection->data);
				for (auto l : time->edges)
				{
					if (l->flow > 0)
					{
						Task* currentTask = findTask(groups, groupNumber, l->connection->data); //dodaj zadanie do kolejki
						tmpTimetable.researcherTasks[i->connection->data].push(currentTask);
					}
				}
			}
		}
	}
	restoreFlowAndCapacity(List, flowBackup, capacityBackup);
	return maxFlow;
}

//szuka katedry na liscie
bool findDepartment(list<Department*> departments, Department* certainDepartment)
{
	for (auto i : departments)
		if (i->name == certainDepartment->name)
			return true;

	return false;
}

//szuka pracownika w katedrze
bool findResearcher(Department* certainDepartment, Researcher* certainResearcher)
{
	for (int i = 0; i < certainDepartment->researcherNumber; i++)
	{
		if (certainDepartment->researchers[i].name == certainResearcher->name)
			return true;
	}
	return false;
}

//liczy wartosc wszystkich pracownikow w harmonogramie i dodaje ich katedry do harmonogramu
void manageTimetable(Timetable& tmpTimetable)
{
	int sum = 0;
	for (auto i : tmpTimetable.researchers)
	{
		sum += i->points;

		if (!(findDepartment(tmpTimetable.departments, i->department)))
		{
			tmpTimetable.departments.push_back(i->department);
			tmpTimetable.departmentResearcherNumbers[i->department->name] = 0;
		}
		tmpTimetable.departmentResearcherNumbers[i->department->name]++;
	}
	tmpTimetable.points = sum;
}

//sprawdza mozliwosc utworzenia harmonogramu dla zestawienia pracownikow
void manageSet(list<Vertex*>& List, queue<Researcher*> Q, list<int> days, list<int> times, int groupNumber, Group* groups, int totalHours, vector<Timetable>& Timetables)
{
	Timetable possibleTimetable;
	while (!Q.empty())
	{
		Researcher* tmp = Q.front();
		Q.pop();

		Vertex* find = findVertex(List, tmp->name);
		if (!find)
		{
			addResearcher(List, *tmp, days, times, groupNumber, groups);
			createSymetricEdges(List);
		}
		possibleTimetable.researchers.push_back(tmp);
	}

	int flowForCurrentNetwork = fordFulkerson(List, possibleTimetable, groupNumber, groups);
	if (flowForCurrentNetwork == totalHours)
	{
		manageTimetable(possibleTimetable);
		Timetables.push_back(possibleTimetable);
	}
}

//rekurencyjnie przechodzi katedry
void manageDepartment(list<Vertex*>& List, Department department, int departmentIndex, Department* departments, int departmentNumber, const list<int>& days, const list<int>& times, int groupNumber, Group* groups, queue<Researcher*> Q, int totalHours, vector<Timetable>& Timetables)
{
	for (int i = 0; i < department.researcherNumber; i++)
	{
		Q.push(&department.researchers[i]);
		manageSet(List, Q, days, times, groupNumber, groups, totalHours, Timetables);

		if (departmentIndex < departmentNumber - 1)
		{
			int departmentsToCheck = departmentNumber - 1 - departmentIndex;

			for (int j = departmentNumber - 1, k = 0; k < departmentsToCheck; j--, k++)
			{
				manageDepartment(List, departments[j], j, departments, departmentNumber, days, times, groupNumber, groups, Q, totalHours, Timetables);
			}
		}
	}
	if (departmentIndex == departmentNumber - 1)
		deleteAllResearchers(List, days, times, groupNumber, groups, departmentNumber, departments);
}

//szereguje pracownikow w trybie pierwszym
void firstInputMode(list<Vertex*>& List, Department* departments, int departmentNumber, const list<int>& days, const list<int>& times, int groupNumber, Group* groups, int totalHours, vector<Timetable>& Timetables)
{
	queue<Researcher*> emptyQueue;
	for (int i = departmentNumber - 1; i >= 0; i--)
	{
		manageDepartment(List, departments[i], i, departments, departmentNumber, days, times, groupNumber, groups, emptyQueue, totalHours, Timetables);
	}
}

//porownuje wartosci wszystkich pracownikow dwoch harmonogramow
bool compareTimetables(const Timetable& first, const Timetable& second)
{
	if (first.points < second.points)
		return true;
	else
		return false;
}

//wypisuje wynik dla pierwszego trybu outputu
void firstOutputMode(vector<Timetable>& Timetables)
{
	cout << Timetables[0].points << endl;
	for (auto i : Timetables[0].departments)
	{
		cout << i->realName << " " << Timetables[0].departmentResearcherNumbers[i->name] << endl;
	}
}

//wypisuje wynik dla drugiego trybu outputu
void secondOutputMode(vector<Timetable>& Timetables)
{
	cout << Timetables[0].points << endl;
	for (auto i : Timetables[0].departments)
	{
		cout << i->realName << " " << Timetables[0].departmentResearcherNumbers[i->name] << endl;

		for (auto j : Timetables[0].researchers)
		{
			if (findResearcher(i, j))
			{
				cout << j->realName << " " << Timetables[0].researcherHours[j->name] << endl;
				while (!(Timetables[0].researcherTasks[j->name].empty()))
				{
					Task* currentTask = Timetables[0].researcherTasks[j->name].front();
					Timetables[0].researcherTasks[j->name].pop();
					if (currentTask)
					{
						cout << currentTask->group->realName << " " << currentTask->day << " " << currentTask->time << endl;
					}
				}
			}
		}
	}
}

int main()
{
	int inputMode, outputMode;
	cin >> inputMode >> outputMode;

	//tryb szeregowania 1
	if (inputMode != 1)
	{
		cout << "Nie obsluguje tego trybu szeregowania" << endl;
		return 0;
	}

	int departmentNumber, groupNumber;
	int totalHours = 0;
	Department* departments; //tablica przechowujaca katedry
	Group* groups; //tablica przechowujaca grupy zadan
	list<int> days; //lista przechowujaca odmienne dni
	list<int> times; //lista przechowujaca odmienne sloty czasu
	list<Vertex*> List; //lista sasiedztwa (sposob reprezentacji sieci przeplywowej)
	vector<Timetable> Timetables; //tablica potencjalnych harmonogramow

	loadDepartments(departmentNumber, departments);
	loadGroups(groupNumber, groups, totalHours, days, times);
	initList(List, groupNumber, groups);

	firstInputMode(List, departments, departmentNumber, days, times, groupNumber, groups, totalHours, Timetables);
	sort(Timetables.begin(), Timetables.end(), compareTimetables);

	switch (outputMode)
	{
	case 1:
		firstOutputMode(Timetables);
		break;
	case 2:
		secondOutputMode(Timetables);
		break;
	}

	//zwolnienie pamieci
	for (int i = 0; i < departmentNumber; i++)
		delete[] departments[i].researchers;
	delete[] departments;
	for (int i = 0; i < groupNumber; i++)
		delete[] groups[i].tasks;
	delete[] groups;

	return 0;
}
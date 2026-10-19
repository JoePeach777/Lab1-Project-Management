
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <locale>

using namespace std;

// =====================================================
// Класс Task — задача проекта.
// Задачи принадлежат проекту: реализована композиция.
// Статическое поле считает количество существующих задач.
// =====================================================
class Task {
private:
    string title;
    bool completed;

    static int taskCount;

public:
    Task(const string& taskTitle)
        : title(taskTitle), completed(false) {
        ++taskCount;
        cout << "[Task] Создана задача: " << title << '\n';
    }

    ~Task() {
        cout << "[Task] Уничтожена задача: " << title << '\n';
        --taskCount;
    }

    void complete() {
        completed = true;
        cout << "Задача \"" << title
            << "\" отмечена выполненной.\n";
    }

    void print() const {
        cout << "  - " << title << " ["
            << (completed ? "выполнена" : "в работе")
            << "]\n";
    }

    static int getCount() {
        return taskCount;
    }
};

// Определение статического поля вне класса.
int Task::taskCount = 0;


// =====================================================
// Класс Project — проект.
// Композиция: Project владеет задачами через unique_ptr.
// При уничтожении проекта задачи уничтожаются автоматически.
// =====================================================
class Project {
private:
    string name;
    vector<unique_ptr<Task>> tasks;

    static int projectCount;

public:
    explicit Project(const string& projectName)
        : name(projectName) {
        ++projectCount;
        cout << "[Project] Создан проект: " << name << '\n';
    }

    // Запрещаем копирование, чтобы исключить неоднозначное
    // владение задачами и ошибки подсчёта объектов.
    Project(const Project&) = delete;
    Project& operator=(const Project&) = delete;

    ~Project() {
        cout << "[Project] Уничтожается проект: "
            << name << '\n';

        // После тела деструктора tasks уничтожится автоматически.
        // Вместе с unique_ptr будут уничтожены и объекты Task.
        --projectCount;
    }

    const string& getName() const {
        return name;
    }

    void addTask(const string& taskTitle) {
        tasks.push_back(make_unique<Task>(taskTitle));
    }

    void print() const {
        cout << "\nПроект: " << name << '\n';

        if (tasks.empty()) {
            cout << "  Задач пока нет.\n";
            return;
        }

        for (const auto& task : tasks) {
            if (task) {
                task->print();
            }
        }
    }

    static int getCount() {
        return projectCount;
    }
};

int Project::projectCount = 0;


// =====================================================
// Класс ProjectManager — Singleton.
// Существует только один экземпляр менеджера.
// Агрегация: менеджер хранит НЕ владеющие указатели Project*.
// Он не создаёт и не удаляет сами проекты.
// =====================================================
class ProjectManager {
private:
    vector<Project*> projects;

    // Приватный конструктор запрещает создание извне.
    ProjectManager() {
        cout << "[Manager] Менеджер проектов создан.\n";
    }

    // Запрещаем копирование и присваивание Singleton.
    ProjectManager(const ProjectManager&) = delete;
    ProjectManager& operator=(const ProjectManager&) = delete;

public:
    // Возвращает ссылку на единственный экземпляр.
    static ProjectManager& getInstance() {
        static ProjectManager instance;
        return instance;
    }

    void addProject(Project* project) {
        // Проверяем nullptr.
        if (project == nullptr) {
            cout << "Ошибка: передан пустой указатель.\n";
            return;
        }

        // Не добавляем один проект дважды.
        if (find(projects.begin(), projects.end(), project)
            != projects.end()) {
            cout << "Проект уже зарегистрирован.\n";
            return;
        }

        projects.push_back(project);
        cout << "[Manager] Зарегистрирован проект: "
            << project->getName() << '\n';
    }

    void removeProject(Project* project) {
        if (project == nullptr) {
            cout << "Ошибка: передан пустой указатель.\n";
            return;
        }

        auto it = find(projects.begin(), projects.end(), project);

        if (it == projects.end()) {
            cout << "Проект не найден в менеджере.\n";
            return;
        }

        projects.erase(it);
        cout << "[Manager] Проект удалён из списка менеджера: "
            << project->getName() << '\n';

        // Сам объект Project НЕ удаляется.
    }

    void printProjects() const {
        cout << "\n=== Список проектов ===\n";

        if (projects.empty()) {
            cout << "Проектов нет.\n";
            return;
        }

        for (const Project* project : projects) {
            if (project != nullptr) {
                project->print();
            }
        }
    }

    size_t getProjectCount() const {
        return projects.size();
    }

    ~ProjectManager() {
        // Проекты здесь не удаляем: менеджер ими не владеет.
        projects.clear();
        cout << "[Manager] Менеджер проектов уничтожен.\n";
    }
};


// =====================================================
// Главная функция — демонстрация всех возможностей.
// =====================================================
int main() {
    setlocale(LC_ALL, "");

    cout << "=== 1. Проверка Singleton ===\n";

    ProjectManager& manager1 = ProjectManager::getInstance();
    ProjectManager& manager2 = ProjectManager::getInstance();

    cout << "Один и тот же экземпляр: "
        << boolalpha << (&manager1 == &manager2) << '\n';

    cout << "\n=== 2. Создание проектов и задач ===\n";
    cout << "Проектов: " << Project::getCount() << '\n';
    cout << "Задач: " << Task::getCount() << '\n';

    Project project1("Разработка сайта");
    project1.addTask("Создать макет");
    project1.addTask("Реализовать интерфейс");

    Project project2("Мобильное приложение");
    project2.addTask("Спроектировать базу данных");

    manager1.addProject(&project1);
    manager1.addProject(&project2);

    manager1.printProjects();

    cout << "\nКоличество проектов: "
        << Project::getCount() << '\n';
    cout << "Количество задач: "
        << Task::getCount() << '\n';

    cout << "\n=== 3. Проверка агрегации ===\n";

    manager1.removeProject(&project1);

    cout << "Проектов в списке менеджера: "
        << manager1.getProjectCount() << '\n';

    // Проект продолжает существовать, хотя его убрали
    // из списка менеджера.
    cout << "Объект проекта всё ещё существует:\n";
    project1.print();

    cout << "\n=== 4. Проверка композиции ===\n";
    cout << "Задач до уничтожения временного проекта: "
        << Task::getCount() << '\n';

    {
        Project temporaryProject("Временный проект");
        temporaryProject.addTask("Временная задача 1");
        temporaryProject.addTask("Временная задача 2");

        cout << "Задач внутри блока: "
            << Task::getCount() << '\n';

        // При выходе из блока проект и его задачи уничтожатся.
    }

    cout << "Задач после уничтожения временного проекта: "
        << Task::getCount() << '\n';

    cout << "\n=== 5. Проверка nullptr ===\n";
    manager1.addProject(nullptr);

    cout << "\n=== 6. Итоговые счётчики ===\n";
    cout << "Существующих проектов: "
        << Project::getCount() << '\n';
    cout << "Существующих задач: "
        << Task::getCount() << '\n';

    cout << "\n=== Завершение программы ===\n";

    return 0;
}

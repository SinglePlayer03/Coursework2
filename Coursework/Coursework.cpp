#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TASKS 32
#define MAX_NAME  5
#define SIMULATION_LIMIT 1000

typedef struct {
    char name[MAX_NAME];
    int period;
    int execution_time;
    int relative_deadline;
    int remaining_time;
    int next_release_time;
    int absolute_deadline;
    int releases;
    int completed;
    int missed_deadlines;
    int active;
} Task;

typedef struct {
    int current_time;
    int hyperperiod;
    int running;
} Scheduler;

Task tasks[MAX_TASKS];
int task_count = 0;

int gcd(int a, int b) {
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

int lcm(int a, int b) {
    return (a * b) / gcd(a, b);
}

int calculate_hyperperiod() {
    if (task_count == 0)
        return 0;

    int hp = tasks[0].period;

    for (int i = 1; i < task_count; i++) {
        hp = lcm(hp, tasks[i].period);
    }

    return hp;
}

void clear_input() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void print_line() {
    printf("--------------------------------------------------------------------\n");
}

void show_task(Task* t, int index) {
    printf(
        "%2d | %-3s | P=%3d | t=%3d | D=%3d\n",
        index,
        t->name,
        t->period,
        t->execution_time,
        t->relative_deadline
    );
}

void list_tasks() {
    if (task_count == 0) {
        printf("Список задач порожній.\n");
        return;
    }

    printf("Поточні задачі:\n");
    print_line();

    for (int i = 0; i < task_count; i++) {
        show_task(&tasks[i], i);
    }
}

void add_task() {
    if (task_count >= MAX_TASKS) {
        printf("Досягнуто максимальну кількість задач.\n");
        return;
    }

    Task t;

    printf("Назва задачі: ");
    scanf_s("%s", t.name, MAX_NAME);

    printf("Період (P): ");
    scanf_s("%d", &t.period);

    printf("Час виконання (t): ");
    scanf_s("%d", &t.execution_time);

    printf("Відносний дедлайн (D): ");
    scanf_s("%d", &t.relative_deadline);

    if (strlen(t.name) > 3) {
        printf("\n");
        print_line();
        printf("Назва задачі не повинна бути довшою за 3 символи.\n");
        return;
    }

    if (t.period <= 0 || t.execution_time <= 0 || t.relative_deadline <= 0 ||
        t.period > 999 || t.execution_time > 999 || t.relative_deadline > 999) {
        printf("\n");
        print_line();
        printf("Параметри повинні бути цілими від 1 до 999\n");
        return;
    }

    t.remaining_time = 0;
    t.next_release_time = 0;
    t.absolute_deadline = 0;
    t.releases = 0;
    t.completed = 0;
    t.missed_deadlines = 0;
    t.active = 0;

    tasks[task_count++] = t;

    printf("Задачу додано.\n");
}

void remove_task() {
    if (task_count == 0) {
        printf("Немає задач для видалення.\n");
        return;
    }

    list_tasks();

    int index;

    print_line();
    printf("Індекс задачі для видалення: ");
    scanf_s("%d", &index);

    if (index < 0 || index >= task_count) {
        printf("Некоректний індекс.\n");
        return;
    }

    for (int i = index; i < task_count - 1; i++) {
        tasks[i] = tasks[i + 1];
    }

    task_count--;

    printf("Задачу видалено.\n");
}

void reset_tasks() {
    for (int i = 0; i < task_count; i++) {
        tasks[i].remaining_time = 0;
        tasks[i].next_release_time = 0;
        tasks[i].absolute_deadline = 0;
        tasks[i].releases = 0;
        tasks[i].completed = 0;
        tasks[i].missed_deadlines = 0;
        tasks[i].active = 0;
    }
}

float cpu_utilization() {
    float u = 0;

    for (int i = 0; i < task_count; i++) {
        u += (float)tasks[i].execution_time / (float)tasks[i].period * 100;
    }

    return u;
}

void release_jobs(int time) {
    for (int i = 0; i < task_count; i++) {

        if (time == tasks[i].next_release_time) {

            if (tasks[i].active && tasks[i].remaining_time > 0) {
                tasks[i].missed_deadlines++;

                printf(
                    "[t=%d] ПРОПУСК ДЕДЛАЙНУ -> %s\n",
                    time,
                    tasks[i].name
                );
            }

            tasks[i].remaining_time = tasks[i].execution_time;
            tasks[i].absolute_deadline = time + tasks[i].relative_deadline;
            tasks[i].next_release_time += tasks[i].period;
            tasks[i].releases++;
            tasks[i].active = 1;

            printf(
                "[t=%d] Доступний -> %s | дедлайн=%d\n",
                time,
                tasks[i].name,
                tasks[i].absolute_deadline
            );
        }
    }
}

int select_edf_task() {
    int selected = -1;

    for (int i = 0; i < task_count; i++) {

        if (tasks[i].active && tasks[i].remaining_time > 0) {

            if (selected == -1) {
                selected = i;
            }
            else if (tasks[i].absolute_deadline < tasks[selected].absolute_deadline) {
                selected = i;
            }
            else if (
                tasks[i].absolute_deadline == tasks[selected].absolute_deadline &&
                tasks[i].period < tasks[selected].period
                ) {
                selected = i;
            }
        }
    }

    return selected;
}

void execute_task(int index, int time) {
    tasks[index].remaining_time--;

    printf(
        "[t=%d] ВИКОНУЄТСЯ -> %-3s | залишилось=%d | дедлайн=%d\n",
        time,
        tasks[index].name,
        tasks[index].remaining_time,
        tasks[index].absolute_deadline
    );

    if (tasks[index].remaining_time == 0) {
        tasks[index].completed++;
        tasks[index].active = 0;

        printf(
            "[t=%d] ВИКОНАНО -> %s\n",
            time + 1,
            tasks[index].name
        );
    }
}

void show_runtime_table() {
    print_line();
    printf("Стан задач:\n");
    print_line();

    for (int i = 0; i < task_count; i++) {
        printf(
            "%-3s | Запущено =%3d | Виконано =%3d | Пропущено дедлайнів =%3d\n",
            tasks[i].name,
            tasks[i].releases,
            tasks[i].completed,
            tasks[i].missed_deadlines
        );
    }
    
    print_line();
    
    for (int i = 0; i < task_count; i++) {
        printf(
            "%-3s | Залишилось t =%3d | Наступний P =%3d | Останній дедлайн =%3d\n",
            tasks[i].name,
            tasks[i].remaining_time,
            tasks[i].next_release_time,
            tasks[i].absolute_deadline
        );
    }
}

void simulate_edf() {

    if (task_count == 0) {
        printf("Немає задач для симуляції.\n");
        return;
    }

    reset_tasks();

    Scheduler scheduler;

    scheduler.current_time = 0;
    scheduler.hyperperiod = calculate_hyperperiod();
    scheduler.running = 1;

    int simulation_time;

    printf(
        "Гіперперіод: %d\n",
        scheduler.hyperperiod
    );

    printf(
        "Тривалість симуляції (max %d): ",
        SIMULATION_LIMIT
    );

    scanf_s("%d", &simulation_time);

    if (simulation_time <= 0 || simulation_time > SIMULATION_LIMIT) {
        printf("Некоректна тривалість симуляції.\n");
        return;
    }

    print_line();
    printf("Старт EDF симуляції\n");
    print_line();

    while (scheduler.current_time < simulation_time && scheduler.running) {

        release_jobs(scheduler.current_time);

        int selected = select_edf_task();

        if (selected == -1) {
            printf(
                "[t=%d] ПРОЦЕСОР ПРОСТОЮЄ\n",
                scheduler.current_time
            );
        }
        else {
            execute_task(selected, scheduler.current_time);
        }

        scheduler.current_time++;
    }

    print_line();
    printf("Кінець симуляції\n");

    show_runtime_table();

    printf(
        "використання процесора: %.f%%\n",
        cpu_utilization()
    );
}

void load_demo_tasks() {

    task_count = 0;

    Task t1 = {
        "T1",
        20,
        3,
        7,
        0,
        0,
        0,
        0,
        0,
        0,
        0
    };

    Task t2 = {
        "T2",
        5,
        2,
        4,
        0,
        0,
        0,
        0,
        0,
        0,
        0
    };

    Task t3 = {
        "T3",
        10,
        2,
        8,
        0,
        0,
        0,
        0,
        0,
        0,
        0
    };

    tasks[0] = t1;
    tasks[1] = t2;
    tasks[2] = t3;

    task_count = 3;

    printf("Демонстраційний набір задач завантажено.\n");
}

void print_menu() {
    print_line();
    printf("Cимулятор алгоритму планування процесів Earliest Deadline First\n");
    print_line();
    printf("1. Додати задачу\n");
    printf("2. Видалити задачу\n");
    printf("3. Показати задачі\n");
    printf("4. Запустити EDF симуляцію\n");
    printf("5. Завантажити демо-набір\n");
    printf("0. Вихід\n");
}

int main() {

    system("chcp 1251 > nul");

    int choice;

    print_menu();

    while (1) {

        print_line();

        printf("Вибір: ");

        if (scanf_s("%d", &choice) != 1) {
            printf("Помилка вводу.\n");
            clear_input();
            continue;
        }

        switch (choice) {

        case 1:
            printf("Додавання задачі\n");
            print_line();
            add_task();
            break;

        case 2:
            printf("Видалення задачі\n");
            print_line();
            remove_task();
            break;

        case 3:
            printf("Показ задачі\n");
            print_line();
            list_tasks();
            break;

        case 4:
            printf("Запуск EDF симуляції\n");
            print_line();
            simulate_edf();
            break;

        case 5:
            printf("Завантаження демо-набору\n");
            print_line();
            load_demo_tasks();
            break;

        case 0:
            printf("Завершення програми.\n");
            return 0;

        default:
            print_line();
            printf("Помилка вводу.\n");
        }
    }

    return 0;
}

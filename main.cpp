#include <QCoreApplication>
#include <QTextStream>
#include <QStringList>
#include "calculator.h"
// Функция для вывода справки по командам
void printHelp(QTextStream& out) {
    out << "Доступные команды:\n";
    out << " add <a> <b> - сложение\n";
    out << " sub <a> <b> - вычитание\n";
    out << " mul <a> <b> - умножение\n";
    out << " div <a> <b> - деление\n";
    out << " reset - сброс\n";
    out << " help - эта справка\n";
    out << " quit - выход\n";
}
int main(int argc, char* argv[]) {
    // QCoreApplication вместо QApplication - для консольного приложения
    QCoreApplication app(argc, argv);
    // Потоки ввода/вывода (кроссплатформенные, поддерживают Unicode)
    QTextStream in(stdin);
    QTextStream out(stdout);
    // Создаём калькулятор (родитель не нужен - живёт до конца программы)
    Calculator calc;
    // СОЕДИНЕНИЯ: связываем сигналы калькулятора с лямбда-обработчиками
    // 1. При успешном вычислении - выводим результат
    QObject::connect(&calc, &Calculator::resultReady,
                     [&out](double result) {
                         out << "Результат: " << result << "\n";
                         out.flush();
                     });
    // 2. При ошибке - выводим сообщение об ошибке
    QObject::connect(&calc, &Calculator::errorOccurred,
                     [&out](const QString& msg) {
                         out << "Ошибка: " << msg << "\n";
                         out.flush();
                     });
    // Приветствие
    out << "=== Консольный калькулятор на Qt ===\n";
    printHelp(out);
    out << "\n> ";out.flush();
    // Основной цикл: читаем строки, парсим, вызываем слоты
    QString line;
    while (in.readLineInto(&line)) {
        line = line.trimmed(); // Убираем пробелы по краям
        // Пустая строка - просто продолжаем
        if (line.isEmpty()) {
            out << "> ";
            out.flush();
            continue;
        }
        // Разбиваем строку на токены по пробелам
        QStringList parts = line.split(' ', Qt::SkipEmptyParts);
        QString command = parts.value(0).toLower();
        // Обработка команд выхода
        if (command == "quit" || command == "exit") {
            out << "До свидания!\n";
            break;
        }
        // Справка
        if (command == "help") {
            printHelp(out);
            out << "> ";
            out.flush();
            continue;
        }
        // Сброс
        if (command == "reset") {
            calc.reset();
            out << "> ";
            out.flush();
            continue;
        }
        // Арифметические команды требуют 3 токена: команда + 2 числа
        if (parts.size() != 3) {
            out << "Ошибка: неверный формат. Используйте: <команда> <a> <b>\n";
            out << "> ";
            out.flush();
            continue;
        }
        // Преобразуем операнды в числа
        bool ok1, ok2;
        double a = parts[1].toDouble(&ok1);
        double b = parts[2].toDouble(&ok2);
        if (!ok1 || !ok2) {
            out << "Ошибка: не удалось преобразовать операнды в числа\n";
            out << "> ";
            out.flush();
            continue;
        }
        // Вызываем нужный слот (обычный вызов метода - сигнал излучится внутри)
        if (command == "add") {
            calc.add(a, b);
        }
        else if (command == "sub") {calc.subtract(a, b);
        }
        else if (command == "mul") {
            calc.multiply(a, b);
        }
        else if (command == "div") {
            calc.divide(a, b);
        }
        else {
            out << "Неизвестная команда: " << command << "\n";
        }
        out << "> ";
        out.flush();
    }
    // QCoreApplication::exec() здесь не нужен:
    // мы работаем в блокирующем режиме чтения, а не через цикл событий.
    // Но если бы использовали QTimer или сеть - обязательно вызвали бы app.exec().
    return 0;
}

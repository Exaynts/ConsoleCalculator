#include <QCoreApplication>
#include <QTextStream>
#include <QStringList>
#include <QSettings>
#include <QFile> // <-- Добавили для работы с файлами
#include "calculator.h"

// Функция для вывода справки по командам
void printHelp(QTextStream& out) {
    out << "Доступные команды:\n";
    out << " add <a> <b> - сложение\n";
    out << " sub <a> <b> - вычитание\n";
    out << " mul <a> <b> - умножение\n";
    out << " div <a> <b> - деление\n";
    out << " prec <n>    - установить точность (кол-во знаков после запятой)\n";
    out << " reset - сброс\n";
    out << " help - эта справка\n";
    out << " quit - выход\n";
}

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    // Настройки для QSettings
    QCoreApplication::setOrganizationName("Lab1Org");
    QCoreApplication::setApplicationName("ConsoleCalculator");

    // Читаем точность из настроек (по умолчанию 2 знака)
    QSettings settings;
    int precision = settings.value("precision", 2).toInt();

    // Потоки ввода/вывода
    QTextStream in(stdin);
    QTextStream out(stdout);

    // Применяем настройки точности
    out.setRealNumberNotation(QTextStream::FixedNotation);
    out.setRealNumberPrecision(precision);

    Calculator calc;

    // --- ШАГ 7: Настройка записи в файл истории ---
    QFile historyFile("history.txt");
    // Открываем файл для записи и добавления в конец (Append)
    if (!historyFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        out << "Предупреждение: Не удалось открыть history.txt для записи.\n";
    }
    QTextStream historyStream(&historyFile);

    // СОЕДИНЕНИЯ

    // 1. Вывод результата в консоль (уже было)
    QObject::connect(&calc, &Calculator::resultReady,
                     [&out](double result) {
                         out << "Результат: " << result << "\n";
                         out.flush();
                     });

    // 2. ШАГ 7: Запись результата в файл истории (новый обработчик)
    QObject::connect(&calc, &Calculator::resultReady,
                     [&historyStream](double result) {
                         historyStream << "Result: " << result << "\n";
                         historyStream.flush();
                     });

    // 3. Вывод ошибок в консоль
    QObject::connect(&calc, &Calculator::errorOccurred,
                     [&out](const QString& msg) {
                         out << "Ошибка: " << msg << "\n";
                         out.flush();
                     });

    // Приветствие
    out << "=== Консольный калькулятор на Qt ===\n";
    out << "Текущая точность: " << precision << " знаков после запятой.\n";
    printHelp(out);
    out << "\n> ";
    out.flush();

    QString line;
    while (in.readLineInto(&line)) {
        line = line.trimmed();
        if (line.isEmpty()) {
            out << "> ";
            out.flush();
            continue;
        }

        QStringList parts = line.split(' ', Qt::SkipEmptyParts);
        QString command = parts.value(0).toLower();

        if (command == "quit" || command == "exit") {
            out << "До свидания!\n";
            break;
        }

        if (command == "help") {
            printHelp(out);
            out << "> ";
            out.flush();
            continue;
        }

        if (command == "reset") {
            calc.reset();
            out << "> ";
            out.flush();
            continue;
        }

        // Обработка команды точности
        if (command == "prec") {
            if (parts.size() != 2) {
                out << "Ошибка: используйте 'prec <число>'\n";
            } else {
                bool ok;
                int newPrec = parts[1].toInt(&ok);
                if (ok && newPrec >= 0 && newPrec <= 15) {
                    precision = newPrec;
                    out.setRealNumberPrecision(precision);
                    settings.setValue("precision", precision);
                    out << "Точность изменена на " << precision << " знаков.\n";
                } else {
                    out << "Ошибка: неверное значение точности (допустимо от 0 до 15)\n";
                }
            }
            out << "> ";
            out.flush();
            continue;
        }

        // Арифметические команды
        if (parts.size() != 3) {
            out << "Ошибка: неверный формат. Используйте: <команда> <a> <b>\n";
            out << "> ";
            out.flush();
            continue;
        }

        bool ok1, ok2;
        double a = parts[1].toDouble(&ok1);
        double b = parts[2].toDouble(&ok2);

        if (!ok1 || !ok2) {
            out << "Ошибка: не удалось преобразовать операнды в числа\n";
            out << "> ";
            out.flush();
            continue;
        }

        if (command == "add") {
            calc.add(a, b);
        }
        else if (command == "sub") {
            calc.subtract(a, b);
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

    // Закрываем файл истории перед выходом
    historyFile.close();

    return 0;
}

// Composition root: the only place that knows every concrete type and wires
// the layers together (infrastructure → application → ui).

#include "application/CalculatorService.h"
#include "infrastructure/JsonHistoryRepository.h"
#include "ui/MainWindow.h"
#include "ui/Theme.h"

#include <QApplication>

#include <memory>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Learning Calculator"));
    QApplication::setOrganizationName(QStringLiteral("Golfam"));
    QApplication::setApplicationVersion(QStringLiteral(CALC_VERSION));

    calc::ui::theme::apply(app);

    auto repository = std::make_unique<calc::infrastructure::JsonHistoryRepository>(
        calc::infrastructure::JsonHistoryRepository::defaultFilePath());
    calc::application::CalculatorService service(std::move(repository));

    calc::ui::MainWindow window(service);
    window.show();

    return QApplication::exec();
}

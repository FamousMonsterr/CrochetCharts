# Crochet Charts

## Русский
`Crochet Charts` — настольное приложение для создания схем вязания крючком. Этот форк приведён в рабочее состояние для современного macOS, переведён на русский язык и собирается на `Qt 5`.

### Что есть в этом форке
- Сборка на современном `cmake` и `Qt 5`.
- Русская локализация интерфейса через `QTranslator`.
- Исправления для современного macOS и свежего toolchain.
- Более чистый старт приложения без прежних warning'ов про `Sans`, `QPainter` и тихую проверку обновлений.
- Режим пониженной графической нагрузки для старых Mac и тяжёлых схем с изображениями.

### Документация
- Подробная инструкция по перемещению, вращению, выравниванию и раскладке петель для бабушкиного квадрата: [docs/granny-square-ru.md](docs/granny-square-ru.md)

### Лицензия
- Исходный код распространяется по `GPLv3`.
- Оригинальные графические материалы и artwork распространяются по `CC BY-SA 4.0`.
- Если вы распространяете модифицированную версию, сохраните и соблюдайте обе лицензии.

Полные тексты лицензий находятся в файле [LICENSE](LICENSE).

### Сборка на macOS
Требования:
- `Xcode Command Line Tools`
- `Homebrew`
- `cmake`
- `qt@5`

Установка зависимостей:

```bash
brew install cmake qt@5
```

Сборка:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/usr/local/opt/qt@5
cmake --build build -j8
```

Создание self-contained `.app`:

```bash
/usr/local/opt/qt@5/bin/macdeployqt build/src/CrochetCharts.app
```

### Запуск
Обычный запуск:

```bash
open build/src/CrochetCharts.app
```

Принудительно включить русский интерфейс:

```bash
CROCHETCHARTS_LANG=ru build/src/CrochetCharts.app/Contents/MacOS/CrochetCharts
```

Если переменная `CROCHETCHARTS_LANG` не задана, приложение включает русский перевод на системах с русской локалью.

### Обновление перевода
Обновить `.ts`:

```bash
lupdate src -ts translations/crochetcharts_ru.ts
```

Собрать `.qm`:

```bash
lrelease translations/crochetcharts_ru.ts -qm translations/crochetcharts_ru.qm
```

### Происхождение
Исходный проект: `StitchworksSoftware/CrochetCharts`.

---

## English
`Crochet Charts` is a desktop application for creating crochet chart patterns. This fork has been updated to build on modern macOS, translated to Russian, and ported to a `Qt 5` based toolchain.

### What This Fork Adds
- Modern `cmake` and `Qt 5` build support.
- Russian UI localization via `QTranslator`.
- Compatibility fixes for modern macOS and current compilers.
- Cleaner startup with the previous `Sans`, `QPainter`, and silent updater warnings removed.
- A lower graphics load mode for older Macs and image-heavy charts.

### Documentation
- Detailed Russian guide for moving, rotating, aligning, and arranging stitches for a granny square workflow: [docs/granny-square-ru.md](docs/granny-square-ru.md)

### License
- Source code is licensed under `GPLv3`.
- Original artwork and graphical assets are licensed under `CC BY-SA 4.0`.
- If you redistribute a modified version, make sure you comply with both licenses.

See [LICENSE](LICENSE) for the full license text shipped with this repository.

### Building on macOS
Requirements:
- `Xcode Command Line Tools`
- `Homebrew`
- `cmake`
- `qt@5`

Install dependencies:

```bash
brew install cmake qt@5
```

Configure and build:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/usr/local/opt/qt@5
cmake --build build -j8
```

Create a self-contained `.app` bundle:

```bash
/usr/local/opt/qt@5/bin/macdeployqt build/src/CrochetCharts.app
```

### Running
Regular launch:

```bash
open build/src/CrochetCharts.app
```

Force Russian UI:

```bash
CROCHETCHARTS_LANG=ru build/src/CrochetCharts.app/Contents/MacOS/CrochetCharts
```

If `CROCHETCHARTS_LANG` is not set, the app enables the Russian translation automatically on systems using a Russian locale.

### Updating Translations
Refresh the `.ts` catalog:

```bash
lupdate src -ts translations/crochetcharts_ru.ts
```

Compile the `.qm` file:

```bash
lrelease translations/crochetcharts_ru.ts -qm translations/crochetcharts_ru.qm
```

### Upstream
Original project: `StitchworksSoftware/CrochetCharts`.

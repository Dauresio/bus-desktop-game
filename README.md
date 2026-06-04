# Bus Jam Puzzle

Jogo de puzzle em Qt6/QML + C++17.  
Objectivo: mover os autocarros coloridos até às plataformas correctas para embarcar todos os passageiros.

---

## Índice

1. [Requisitos](#1-requisitos)
2. [Estrutura do projecto](#2-estrutura-do-projecto)
3. [Antes de compilar](#3-antes-de-compilar)
4. [Compilar — Desktop](#4-compilar--desktop)
5. [Compilar — Android](#5-compilar--android)
6. [Compilar — iOS](#6-compilar--ios)
7. [Executar](#7-executar)
8. [Testes unitários](#8-testes-unitários)
9. [Adicionar níveis](#9-adicionar-níveis)
10. [Onde verificar erros](#10-onde-verificar-erros)
11. [Erros comuns e soluções](#11-erros-comuns-e-soluções)

---

## 1. Requisitos

| Componente | Versão mínima | Notas |
|---|---|---|
| **Qt** | 6.4 | Módulos: Core, Gui, Quick, Qml, QuickControls2, QuickLayouts |
| **CMake** | 3.21 | Incluído no Qt Online Installer |
| **Compilador C++** | C++17 | GCC 10+, Clang 12+, MSVC 2019+ |
| **Ninja** | qualquer | Opcional mas recomendado (`-G Ninja`) |

> **Onde obter o Qt:** https://www.qt.io/download-qt-installer  
> Seleccionar: *Qt 6.x → Desktop (gcc_64 / MSVC / clang)* + *Qt Quick* + *Qt Quick Controls*

---

## 2. Estrutura do projecto

```
BusJamPuzzle/
├── CMakeLists.txt          ← build system
├── main.cpp                ← ponto de entrada
├── README.md               ← este ficheiro
│
├── src/
│   ├── common/
│   │   └── Enums.h         ← enums partilhados (BusColor, Direction, GameState…)
│   ├── model/              ← entidades de domínio puras (sem Qt)
│   │   ├── Bus.h/.cpp
│   │   ├── Passenger.h/.cpp
│   │   ├── Platform.h/.cpp
│   │   └── Board.h/.cpp
│   ├── engine/             ← lógica de jogo + modelos QML
│   │   ├── BoardEngine.h/.cpp
│   │   ├── MovementSolver.h/.cpp
│   │   ├── GameController.h/.cpp
│   │   ├── BusListModel.h/.cpp
│   │   ├── PassengerListModel.h/.cpp
│   │   └── PlatformListModel.h/.cpp
│   └── persistence/        ← I/O: níveis JSON, progresso, QSettings
│       ├── LevelLoader.h/.cpp
│       ├── LevelManager.h/.cpp
│       └── PersistenceManager.h/.cpp
│
├── qml/
│   ├── main.qml            ← raiz da aplicação (Window + StackView)
│   ├── screens/
│   │   ├── MainMenuScreen.qml
│   │   ├── LevelSelectScreen.qml
│   │   └── GameScreen.qml
│   ├── components/
│   │   ├── BoardView.qml   ← tabuleiro completo
│   │   ├── BusItem.qml     ← autocarro visual
│   │   ├── PassengerItem.qml
│   │   ├── PlatformView.qml
│   │   ├── HUD.qml
│   │   └── qmldir
│   └── style/
│       ├── Theme.qml       ← cores, fontes, animações (singleton)
│       ├── Sizes.qml       ← dimensões responsivas (singleton)
│       └── qmldir
│
├── levels/                 ← ficheiros JSON dos 10 níveis incluídos
│   ├── level_001.json  (dificuldade 1 — tutorial)
│   ├── level_002.json  (dificuldade 2)
│   └── … level_010.json (dificuldade 5 — 10×10, tempo 90s)
│
├── assets/
│   ├── fonts/              ← coloque aqui FredokaOne-Regular.ttf e Nunito*.ttf
│   └── icons/              ← coloque aqui app_icon.png
│
└── tests/
    ├── tst_BoardEngine.cpp
    ├── tst_MovementSolver.cpp
    └── tst_LevelLoader.cpp
```

---

## 3. Antes de compilar

### 3.1 Fontes (opcional mas recomendado)

O jogo usa as fontes **Fredoka One** e **Nunito** para o estilo cartoon.  
Sem elas o jogo funciona com a fonte do sistema, mas perde o visual pretendido.

1. Descarregar de Google Fonts (licença OFL — uso livre):
   - https://fonts.google.com/specimen/Fredoka+One → `FredokaOne-Regular.ttf`
   - https://fonts.google.com/specimen/Nunito → `Nunito-Regular.ttf` e `Nunito-Bold.ttf`
2. Copiar para `assets/fonts/`

### 3.2 Ícone (opcional)

Colocar um `app_icon.png` (512×512 px) em `assets/icons/`.  
Sem este ficheiro o CMake emite apenas um aviso — não é erro fatal.

### 3.3 Verificar instalação do Qt

```bash
# Deve mostrar a versão do qmake (confirma que o Qt está no PATH)
qmake --version

# Ou verificar directamente o cmake config
cmake --find-package -DNAME=Qt6 -DCOMPILER_ID=GNU -DLANGUAGE=CXX -DMODE=EXIST
```

---

## 4. Compilar — Desktop

### Linux / macOS

```bash
# 1. Entrar na pasta do projecto
cd BusJamPuzzle

# 2. Configurar (substituir /opt/Qt/6.x/gcc_64 pelo caminho real)
cmake -B build \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH=/opt/Qt/6.x/gcc_64

# Com Ninja (mais rápido):
cmake -B build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH=/opt/Qt/6.x/gcc_64

# 3. Compilar (usar todos os cores disponíveis)
cmake --build build --parallel

# Ou com Ninja:
ninja -C build
```

### Windows (MSVC)

Abrir *Developer Command Prompt for VS 2019/2022*:

```cmd
cd BusJamPuzzle
cmake -B build -G "Visual Studio 17 2022" -A x64 ^
      -DCMAKE_PREFIX_PATH=C:\Qt\6.x\msvc2019_64

cmake --build build --config Release
```

### macOS (Apple Silicon)

```bash
cmake -B build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH=~/Qt/6.x/macos \
      -DCMAKE_OSX_ARCHITECTURES=arm64

cmake --build build --parallel
```

---

## 5. Compilar — Android

```bash
# Variáveis de ambiente necessárias
export ANDROID_SDK_ROOT=$HOME/Android/Sdk
export ANDROID_NDK_ROOT=$ANDROID_SDK_ROOT/ndk/25.x.x

cmake -B build-android \
      -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=arm64-v8a \
      -DANDROID_PLATFORM=android-21 \
      -DCMAKE_PREFIX_PATH=~/Qt/6.x/android_arm64_v8a \
      -DCMAKE_BUILD_TYPE=Release

cmake --build build-android --target apk

# APK gerado em:
# build-android/android-build/build/outputs/apk/debug/android-build-debug.apk
```

---

## 6. Compilar — iOS

Requer macOS com Xcode instalado:

```bash
cmake -B build-ios -G Xcode \
      -DCMAKE_SYSTEM_NAME=iOS \
      -DCMAKE_PREFIX_PATH=~/Qt/6.x/ios \
      -DCMAKE_BUILD_TYPE=Release

cmake --build build-ios --config Release -- -sdk iphonesimulator

# Para dispositivo real: abrir build-ios/BusJamPuzzle.xcodeproj no Xcode,
# configurar signing e seleccionar o dispositivo.
```

---

## 7. Executar

### Desktop — directamente

```bash
# Linux/macOS
./build/BusJamPuzzle

# Windows
.\build\Release\BusJamPuzzle.exe
```

### Desktop — com níveis externos (desenvolvimento)

Se os níveis não estiverem nos recursos Qt, o jogo procura
automaticamente uma pasta `levels/` junto ao executável:

```bash
# Copiar os níveis para junto do executável
cp -r levels/ build/

./build/BusJamPuzzle
```

### Com output de debug visível

```bash
# Linux/macOS — mostrar todos os qDebug no terminal
QT_LOGGING_RULES="*.debug=true" ./build/BusJamPuzzle

# Windows
set QT_LOGGING_RULES=*.debug=true
.\build\Release\BusJamPuzzle.exe
```

### Android — instalar no dispositivo

```bash
adb install build-android/android-build/build/outputs/apk/debug/android-build-debug.apk
adb shell am start -n com.example.BusJamPuzzle/.BusJamPuzzleActivity
```

---

## 8. Testes unitários

```bash
# Compilar com testes activados
cmake -B build-test \
      -DCMAKE_BUILD_TYPE=Debug \
      -DBUSJAM_BUILD_TESTS=ON \
      -DCMAKE_PREFIX_PATH=/opt/Qt/6.x/gcc_64

cmake --build build-test --parallel

# Executar todos os testes
cd build-test && ctest --output-on-failure

# Executar um teste específico com output detalhado
./tst_BoardEngine -v2
./tst_MovementSolver -v2
./tst_LevelLoader -v2
```

Os esqueletos dos testes estão em `tests/`. O conteúdo real deve ser implementado
com `QVERIFY`, `QCOMPARE` e `QTEST_MAIN` usando Qt Test.

---

## 9. Adicionar níveis

1. Criar `levels/level_NNN.json` seguindo este esquema mínimo:

```json
{
  "schemaVersion": 1,
  "id": 11,
  "name": "Nome do Nível",
  "difficulty": 3,
  "rows": 8,
  "cols": 8,
  "timeLimitSecs": 0,
  "buses": [
    { "id": 0, "color": "Red", "capacity": 4, "row": 3, "col": 0, "direction": "Right" }
  ],
  "passengers": [
    { "id": 0, "color": "Red", "row": 3, "col": 5 }
  ],
  "platforms": [
    { "id": 0, "color": "Red",    "row": 3, "col": 7, "exitDirection": "Right" },
    { "id": 1, "color": "Blue",   "row": 0, "col": 7, "exitDirection": "Right" },
    { "id": 2, "color": "Green",  "row": 6, "col": 7, "exitDirection": "Right" },
    { "id": 3, "color": "Yellow", "row": 7, "col": 7, "exitDirection": "Right" }
  ]
}
```

   **Regras obrigatórias:**
   - `id` único e sequencial
   - `capacity` ∈ {4, 6, 8, 12}
   - `color` ∈ {"Red", "Blue", "Green", "Yellow", "Purple"}
   - `direction` / `exitDirection` ∈ {"Up", "Down", "Left", "Right"}
   - Nenhuma entidade pode ocupar a mesma célula
   - Número de plataformas: entre 4 e 8
   - Todas as posições dentro de `[0, rows[` × `[0, cols[`

2. Adicionar ao `CMakeLists.txt` na secção `qt_add_resources`:

```cmake
qt_add_resources(BusJamPuzzle "level_data"
    PREFIX "/levels"
    FILES
        levels/level_001.json
        ...
        levels/level_011.json   # ← adicionar aqui
)
```

3. Recompilar.

---

## 10. Onde verificar erros

### Durante a configuração CMake

```
CMake Error: Could not find Qt6
```
→ Verificar o caminho em `-DCMAKE_PREFIX_PATH`. Deve apontar para a pasta
que contém `lib/cmake/Qt6/Qt6Config.cmake`.

```
CMake Warning: Qt6::QuickLayouts not found
```
→ No Qt Online Installer, garantir que o componente *Qt Quick* está instalado
para a versão e plataforma seleccionadas.

---

### Durante a compilação C++

Os erros mais comuns e onde aparecem:

| Ficheiro | Causa frequente |
|---|---|
| `GameController.cpp` | `LevelManager.h` ou `PersistenceManager.h` não encontrado → verificar `target_include_directories` no `CMakeLists.txt` |
| `BusListModel.cpp` | `Board.h` não encontrado → garantir `src/model` nos includes |
| `BoardEngine.cpp` | `Bus.h` não encontrado → garantir `src/model` nos includes |

Activar output verboso para ver o comando de compilação completo:

```bash
cmake --build build --parallel -- VERBOSE=1
# ou com Ninja:
ninja -C build -v
```

---

### Em runtime — QML

Erros de QML aparecem na consola do terminal ao lançar o executável.

```
QQmlApplicationEngine failed to load component
qrc:/BusJamPuzzle/qml/main.qml:5:1: module "BusJamPuzzle.style" is not installed
```
→ Os ficheiros `qmldir` não estão a ser incluídos. Verificar a secção
`RESOURCES` do `qt_add_qml_module` no `CMakeLists.txt`.

```
TypeError: Cannot read property 'busId' of undefined
```
→ O `Repeater` está a tentar aceder a um role que não existe no modelo.
Verificar os nomes dos roles em `BusListModel::roleNames()`.

```
ReferenceError: gameController is not defined
```
→ O `setContextProperty("gameController", ...)` em `main.cpp` não foi chamado
antes de `engine.load(...)`.

Para activar logs de QML detalhados:

```bash
QML_IMPORT_TRACE=1 QT_LOGGING_RULES="qml=true" ./build/BusJamPuzzle 2>&1 | tee qml.log
```

---

### Em runtime — dados

Os ficheiros de progresso e settings são guardados em:

| SO | Caminho |
|---|---|
| Linux | `~/.local/share/BusJamStudio/BusJamPuzzle/` |
| macOS | `~/Library/Application Support/BusJamStudio/BusJamPuzzle/` |
| Windows | `%APPDATA%\BusJamStudio\BusJamPuzzle\` |
| Android | `/data/data/com.example.BusJamPuzzle/` |

Para repor o progresso ao estado inicial (apagar dados guardados):

```bash
# Linux
rm -rf ~/.local/share/BusJamStudio/BusJamPuzzle/

# macOS
rm -rf ~/Library/Application\ Support/BusJamStudio/BusJamPuzzle/

# Windows (PowerShell)
Remove-Item -Recurse "$env:APPDATA\BusJamStudio\BusJamPuzzle"
```

Ou a partir do código: `persistenceManager.resetAll()`.

---

### Android — logcat

```bash
# Ver logs do jogo em tempo real
adb logcat -s Qt BusJamPuzzle

# Filtrar apenas erros
adb logcat "*:E" | grep -i busjam

# Capturar crash completo
adb logcat -c && adb logcat > logcat.txt
```

---

## 11. Erros comuns e soluções

### "No levels found" ao iniciar

O `LevelManager` tenta três localizações por ordem:
1. Recursos Qt (`:/levels/`)
2. `levels/` junto ao executável
3. `levels/` no directório de trabalho actual

**Solução rápida em desenvolvimento:**
```bash
# Copiar os níveis para junto do executável
cp -r levels/ build/
./build/BusJamPuzzle
```

**Solução para produção:**  
Garantir que os ficheiros JSON estão listados em `qt_add_resources` no `CMakeLists.txt`.

---

### Autocarro não se move ao clicar

Verificar no terminal se aparece:

```
GameController::onBusTapped: input ignorado (estado=3)
```
→ O jogo está pausado (estado 3 = `Paused`). Normal.

```
BoardEngine::requestBusMove id=0 → Blocked
```
→ O caminho está bloqueado. Comportamento correcto do jogo.

```
BoardEngine::requestBusMove id=0 → NoPlatform
```
→ Não existe plataforma da cor deste autocarro na sua direcção.
Verificar a definição do nível JSON.

---

### Crash ao carregar um nível

Activar build Debug e consultar o stack trace:

```bash
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_PREFIX_PATH=/opt/Qt/6.x/gcc_64

cmake --build build-debug --parallel

# Linux — com AddressSanitizer para detectar corrupção de memória
ASAN_OPTIONS=detect_leaks=1 ./build-debug/BusJamPuzzle
```

O `LevelLoader` valida o JSON antes de criar qualquer objecto. Se o crash
ocorrer durante `LevelLoader::fromFile()`, o erro estará nos logs como:

```
[level_005.json] buses[2].capacity: Capacidade 5 inválida. Valores: 4, 6, 8, 12.
```

---

### Fontes não carregadas (texto quadrado no jogo)

```
Fonte não carregada: ":/fonts/FredokaOne-Regular.ttf"
```

→ Os ficheiros `.ttf` não estão em `assets/fonts/` ou não foram adicionados
ao `qt_add_resources` no `CMakeLists.txt`. O jogo funciona com a fonte de sistema.

---

### QML: componente não encontrado

```
module "BusJamPuzzle.components" is not installed
```

→ O `qmldir` em `qml/components/` não está listado nos `RESOURCES` do
`qt_add_qml_module`. Verificar o `CMakeLists.txt`.

---

## Licença

Código produzido como demonstração de arquitectura Qt6/QML + C++17.  
Fontes externas (Fredoka One, Nunito) têm licença SIL Open Font License 1.1.

---

*Bus Jam Puzzle — v1.0.0*

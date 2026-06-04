/**
 * @file Board.cpp
 * @brief Implementação da classe Board para o jogo Bus Jam Puzzle.
 *
 * Todos os invariantes descritos em Board.h são mantidos aqui.
 * Cada método que muta a grelha de ocupação atualiza AMBAS as grelhas
 * (occupancyGrid_ e idGrid_) atomicamente, nunca uma sem a outra.
 *
 * Política de erro:
 *  - Erros de configuração (dimensões inválidas): std::invalid_argument no ctor.
 *  - Violações de pré-condição em runtime: qWarning + return false/nullptr.
 *  - Erros de programação verificáveis: Q_ASSERT (apenas em debug builds).
 */

#include "Board.h"

#include <QDebug>
#include <stdexcept>
#include <algorithm>   // std::none_of, std::all_of, std::count_if

// ─────────────────────────────────────────────────────────────────────────────
// Construtor
// ─────────────────────────────────────────────────────────────────────────────

Board::Board(int rows, int cols)
    : rows_{ rows }
    , cols_{ cols }
{
    if (rows_ < BoardConstants::kMinRows || rows_ > BoardConstants::kMaxRows)
    {
        throw std::invalid_argument(
            QString("Board: número de linhas inválido (%1). "
                    "Intervalo permitido: [%2, %3].")
                .arg(rows_)
                .arg(BoardConstants::kMinRows)
                .arg(BoardConstants::kMaxRows)
                .toStdString());
    }

    if (cols_ < BoardConstants::kMinCols || cols_ > BoardConstants::kMaxCols)
    {
        throw std::invalid_argument(
            QString("Board: número de colunas inválido (%1). "
                    "Intervalo permitido: [%2, %3].")
                .arg(cols_)
                .arg(BoardConstants::kMinCols)
                .arg(BoardConstants::kMaxCols)
                .toStdString());
    }

    clearGrids();

    qDebug().noquote()
        << QString("Board criado: %1x%2.").arg(rows_).arg(cols_);
}

// ─────────────────────────────────────────────────────────────────────────────
// Dimensões
// ─────────────────────────────────────────────────────────────────────────────

int Board::rows() const noexcept { return rows_; }
int Board::cols() const noexcept { return cols_; }

bool Board::isValidPosition(QPoint pos) const noexcept
{
    return pos.x() >= 0 && pos.x() < cols_
        && pos.y() >= 0 && pos.y() < rows_;
}

bool Board::isValidPosition(int row, int col) const noexcept
{
    return isValidPosition(QPoint(col, row));
}

// ─────────────────────────────────────────────────────────────────────────────
// Adição de entidades
// ─────────────────────────────────────────────────────────────────────────────

bool Board::addBus(std::unique_ptr<Bus> bus)
{
    if (!bus)
    {
        qWarning() << "Board::addBus: ponteiro nulo recebido.";
        return false;
    }

    if (isDuplicateId(buses_, bus->id()))
    {
        qWarning().noquote()
            << "Board::addBus: já existe um Bus com id=" << bus->id();
        return false;
    }

    const QPoint pos = bus->position();

    if (!canOccupyCell(pos, "addBus"))
        return false;

    // Ocupa a célula antes de transferir a posse, para que o id seja
    // consultável via bus->id() ainda dentro deste scope.
    const int id = bus->id();
    occupancyGrid_[pos.y()][pos.x()] = OccupancyType::BusEntity;
    idGrid_[pos.y()][pos.x()]        = id;

    buses_.push_back(std::move(bus));

    qDebug().noquote()
        << "Board: Bus id=" << id << "adicionado em" << pos;

    return true;
}

bool Board::addPassenger(std::unique_ptr<Passenger> passenger)
{
    if (!passenger)
    {
        qWarning() << "Board::addPassenger: ponteiro nulo recebido.";
        return false;
    }

    if (isDuplicateId(passengers_, passenger->id()))
    {
        qWarning().noquote()
            << "Board::addPassenger: já existe um Passenger com id="
            << passenger->id();
        return false;
    }

    const QPoint pos = passenger->cellPosition();

    if (!canOccupyCell(pos, "addPassenger"))
        return false;

    const int id = passenger->id();
    occupancyGrid_[pos.y()][pos.x()] = OccupancyType::Passenger;
    idGrid_[pos.y()][pos.x()]        = id;

    passengers_.push_back(std::move(passenger));

    qDebug().noquote()
        << "Board: Passenger id=" << id << "adicionado em" << pos;

    return true;
}

bool Board::addPlatform(std::unique_ptr<Platform> platform)
{
    if (!platform)
    {
        qWarning() << "Board::addPlatform: ponteiro nulo recebido.";
        return false;
    }

    if (isDuplicateId(platforms_, platform->id()))
    {
        qWarning().noquote()
            << "Board::addPlatform: já existe uma Platform com id="
            << platform->id();
        return false;
    }

    const QPoint pos = platform->position();

    if (!canOccupyCell(pos, "addPlatform"))
        return false;

    const int id = platform->id();
    occupancyGrid_[pos.y()][pos.x()] = OccupancyType::Platform;
    idGrid_[pos.y()][pos.x()]        = id;

    platforms_.push_back(std::move(platform));

    qDebug().noquote()
        << "Board: Platform id=" << id << "adicionada em" << pos;

    return true;
}

bool Board::validatePlatformCount() const
{
    const int count = platformCount();

    if (count < BoardConstants::kMinPlatforms || count > BoardConstants::kMaxPlatforms)
    {
        qWarning().noquote()
            << "Board::validatePlatformCount: número de plataformas inválido ("
            << count << ")."
            << "Intervalo permitido: ["
            << BoardConstants::kMinPlatforms << ","
            << BoardConstants::kMaxPlatforms << "].";
        return false;
    }

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Contagem de entidades
// ─────────────────────────────────────────────────────────────────────────────

int Board::busCount()       const noexcept { return buses_.size(); }
int Board::passengerCount() const noexcept { return passengers_.size(); }
int Board::platformCount()  const noexcept { return platforms_.size(); }

// ─────────────────────────────────────────────────────────────────────────────
// Acesso por id
// ─────────────────────────────────────────────────────────────────────────────

Bus* Board::findBusById(int id)
{
    return findById(buses_, id);
}

const Bus* Board::findBusById(int id) const
{
    return findById(buses_, id);
}

Passenger* Board::findPassengerById(int id)
{
    return findById(passengers_, id);
}

const Passenger* Board::findPassengerById(int id) const
{
    return findById(passengers_, id);
}

Platform* Board::findPlatformById(int id)
{
    return findById(platforms_, id);
}

const Platform* Board::findPlatformById(int id) const
{
    return findById(platforms_, id);
}

// ─────────────────────────────────────────────────────────────────────────────
// Acesso por posição
// ─────────────────────────────────────────────────────────────────────────────

CellInfo Board::cellInfoAt(QPoint pos) const noexcept
{
    if (!isValidPosition(pos))
        return CellInfo{};

    return CellInfo{
        occupancyGrid_[pos.y()][pos.x()],
        idGrid_[pos.y()][pos.x()]
    };
}

CellInfo Board::cellInfoAt(int row, int col) const noexcept
{
    return cellInfoAt(QPoint(col, row));
}

bool Board::isCellFree(QPoint pos) const noexcept
{
    if (!isValidPosition(pos))
        return false;  // Posição fora dos limites não é "livre" para ocupação.

    return occupancyGrid_[pos.y()][pos.x()] == OccupancyType::None;
}

bool Board::isCellFree(int row, int col) const noexcept
{
    return isCellFree(QPoint(col, row));
}

Bus* Board::busAt(QPoint pos)
{
    const CellInfo info = cellInfoAt(pos);

    if (info.type != OccupancyType::BusEntity)
        return nullptr;

    return findBusById(info.id);
}

const Bus* Board::busAt(QPoint pos) const
{
    const CellInfo info = cellInfoAt(pos);

    if (info.type != OccupancyType::BusEntity)
        return nullptr;

    return findBusById(info.id);
}

Passenger* Board::passengerAt(QPoint pos)
{
    const CellInfo info = cellInfoAt(pos);

    if (info.type != OccupancyType::Passenger)
        return nullptr;

    return findPassengerById(info.id);
}

const Passenger* Board::passengerAt(QPoint pos) const
{
    const CellInfo info = cellInfoAt(pos);

    if (info.type != OccupancyType::Passenger)
        return nullptr;

    return findPassengerById(info.id);
}

Platform* Board::platformAt(QPoint pos)
{
    const CellInfo info = cellInfoAt(pos);

    if (info.type != OccupancyType::Platform)
        return nullptr;

    return findPlatformById(info.id);
}

const Platform* Board::platformAt(QPoint pos) const
{
    const CellInfo info = cellInfoAt(pos);

    if (info.type != OccupancyType::Platform)
        return nullptr;

    return findPlatformById(info.id);
}

// ─────────────────────────────────────────────────────────────────────────────
// Acesso às coleções completas
// ─────────────────────────────────────────────────────────────────────────────

const std::vector<std::unique_ptr<Bus>>& Board::buses() const noexcept
{
    return buses_;
}

const std::vector<std::unique_ptr<Passenger>>& Board::passengers() const noexcept
{
    return passengers_;
}

const std::vector<std::unique_ptr<Platform>>& Board::platforms() const noexcept
{
    return platforms_;
}

// ─────────────────────────────────────────────────────────────────────────────
// Gestão da grelha de ocupação
// ─────────────────────────────────────────────────────────────────────────────

bool Board::occupyCell(QPoint pos, OccupancyType type, int id)
{
    if (!isValidPosition(pos))
    {
        qWarning().noquote()
            << "Board::occupyCell: posição fora dos limites" << pos;
        return false;
    }

    if (occupancyGrid_[pos.y()][pos.x()] != OccupancyType::None)
    {
        qWarning().noquote()
            << "Board::occupyCell: célula" << pos
            << "já está ocupada (id="
            << idGrid_[pos.y()][pos.x()] << ").";
        return false;
    }

    // Actualiza ambas as grelhas atomicamente.
    occupancyGrid_[pos.y()][pos.x()] = type;
    idGrid_[pos.y()][pos.x()]        = id;

    return true;
}

bool Board::freeCell(QPoint pos)
{
    if (!isValidPosition(pos))
    {
        qWarning().noquote()
            << "Board::freeCell: posição fora dos limites" << pos;
        return false;
    }

    if (occupancyGrid_[pos.y()][pos.x()] == OccupancyType::None)
    {
        qWarning().noquote()
            << "Board::freeCell: célula" << pos << "já estava livre.";
        return false;
    }

    // Plataformas são estruturais — nunca devem ser libertadas via freeCell.
    // O BoardEngine gere o estado da plataforma via Platform::liberarPlatform().
    if (occupancyGrid_[pos.y()][pos.x()] == OccupancyType::Platform)
    {
        qWarning().noquote()
            << "Board::freeCell: tentativa de libertar célula de plataforma em"
            << pos << ". Operação recusada.";
        return false;
    }

    occupancyGrid_[pos.y()][pos.x()] = OccupancyType::None;
    idGrid_[pos.y()][pos.x()]        = -1;

    return true;
}

bool Board::moveBusOnGrid(int busId, QPoint fromPos, QPoint toPos)
{
    // Valida origem: deve estar marcada com este Bus.
    if (!isValidPosition(fromPos))
    {
        qWarning().noquote()
            << "Board::moveBusOnGrid: posição de origem inválida" << fromPos;
        return false;
    }

    if (occupancyGrid_[fromPos.y()][fromPos.x()] != OccupancyType::BusEntity
     || idGrid_[fromPos.y()][fromPos.x()] != busId)
    {
        qWarning().noquote()
            << "Board::moveBusOnGrid: origem" << fromPos
            << "não corresponde ao Bus id=" << busId
            << ". tipo=" << static_cast<int>(occupancyGrid_[fromPos.y()][fromPos.x()])
            << "idNaGrelha=" << idGrid_[fromPos.y()][fromPos.x()];
        return false;
    }

    // Valida destino: deve ser válido e não estar ocupado por outra entidade.
    // Plataformas são a exceção: o Bus pode mover-se para uma célula de plataforma
    // (isso representa a chegada à plataforma, não uma colisão).
    if (!isValidPosition(toPos))
    {
        qWarning().noquote()
            << "Board::moveBusOnGrid: posição de destino inválida" << toPos;
        return false;
    }

    const OccupancyType destType = occupancyGrid_[toPos.y()][toPos.x()];

    if (destType != OccupancyType::None && destType != OccupancyType::Platform)
    {
        qWarning().noquote()
            << "Board::moveBusOnGrid: célula de destino" << toPos
            << "está ocupada por tipo="
            << static_cast<int>(destType)
            << "id=" << idGrid_[toPos.y()][toPos.x()];
        return false;
    }

    // Liberta a origem.
    occupancyGrid_[fromPos.y()][fromPos.x()] = OccupancyType::None;
    idGrid_[fromPos.y()][fromPos.x()]        = -1;

    // Ocupa o destino (sobrepõe plataforma se for o caso — a plataforma
    // permanece registada em platforms_, o Bus fica "sobre" ela na grelha).
    occupancyGrid_[toPos.y()][toPos.x()] = OccupancyType::BusEntity;
    idGrid_[toPos.y()][toPos.x()]        = busId;

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Regras de movimento
// ─────────────────────────────────────────────────────────────────────────────

bool Board::canBusAdvance(int busId) const
{
    const Bus* bus = findBusById(busId);

    if (!bus)
    {
        qWarning().noquote()
            << "Board::canBusAdvance: Bus id=" << busId << "não encontrado.";
        return false;
    }

    if (!bus->isActive() || bus->hasLeft())
        return false;

    const QPoint next = bus->nextPosition();

    // O destino deve estar dentro dos limites.
    if (!isValidPosition(next))
        return false;

    const OccupancyType cellType = occupancyGrid_[next.y()][next.x()];

    // Pode avançar para: célula vazia ou plataforma da mesma cor.
    if (cellType == OccupancyType::None)
        return true;

    if (cellType == OccupancyType::Platform)
    {
        const int platformId = idGrid_[next.y()][next.x()];
        const Platform* platform = findPlatformById(platformId);

        // Só avança para a plataforma se a cor coincidir e estiver livre.
        return platform
            && platform->color() == bus->color()
            && platform->isFree();
    }

    // Passageiros da mesma cor são recolhidos em trânsito (não bloqueiam).
    if (cellType == OccupancyType::Passenger)
    {
        const int passId = idGrid_[next.y()][next.x()];
        const Passenger* p = findPassengerById(passId);
        return p && p->color() == bus->color() && p->isWaiting();
    }

    // Célula ocupada por outro Bus ou passageiro de outra cor: bloqueado.
    return false;
}

bool Board::isPathToplatformClear(int busId) const
{
    const Bus* bus = findBusById(busId);

    if (!bus || !bus->isActive() || bus->hasLeft())
        return false;

    const Platform* target = findTargetPlatform(busId);

    if (!target)
        return false;

    // Percorre todas as células entre a posição atual e a plataforma.
    // A direção do Bus determina o eixo de movimento.
    const QPoint busPos      = bus->position();
    const QPoint platformPos = target->position();
    const QPoint delta       = QPoint(
        (platformPos.x() > busPos.x()) ? 1 : (platformPos.x() < busPos.x()) ? -1 : 0,
        (platformPos.y() > busPos.y()) ? 1 : (platformPos.y() < busPos.y()) ? -1 : 0
    );

    // Garante que o delta é coerente com a direção do Bus.
    // (O MovementSolver fará verificações mais rigorosas.)
    QPoint current = busPos + delta;  // Começa na célula SEGUINTE à posição atual.

    while (current != platformPos)
    {
        if (!isValidPosition(current))
            return false;  // Caminho sai dos limites — não deveria acontecer.

        const OccupancyType cellType = occupancyGrid_[current.y()][current.x()];

        if (cellType == OccupancyType::None)
        {
            current += delta;
            continue;
        }

        // Passageiros da mesma cor são recolhidos em trânsito — não bloqueiam.
        if (cellType == OccupancyType::Passenger)
        {
            const int passId = idGrid_[current.y()][current.x()];
            const Passenger* p = findPassengerById(passId);
            if (p && p->color() == bus->color() && p->isWaiting())
            {
                current += delta;
                continue;
            }
        }

        // Qualquer outra entidade no caminho (Bus, outra plataforma,
        // passageiro de cor diferente) bloqueia.
        return false;
    }

    // A célula da plataforma em si deve estar livre para o autocarro entrar.
    return target->isFree();
}

const Platform* Board::findTargetPlatform(int busId) const
{
    const Bus* bus = findBusById(busId);

    if (!bus)
        return nullptr;

    const BusColor busColor   = bus->color();
    const Direction busDir    = bus->direction();

    for (const auto& platformPtr : std::as_const(platforms_))
    {
        const Platform* platform = platformPtr.get();

        // A plataforma deve ter a mesma cor e a direção de saída oposta
        // (o Bus move-se para ela, ela "aponta" para fora do tabuleiro).
        if (platform->color() != busColor)
            continue;

        // Verifica alinhamento: a plataforma deve estar na direção do Bus
        // a partir da sua posição atual.
        const QPoint busPos      = bus->position();
        const QPoint platformPos = platform->position();

        bool aligned = false;

        switch (busDir)
        {
            case Direction::Right: aligned = (platformPos.y() == busPos.y() && platformPos.x() > busPos.x()); break;
            case Direction::Left:  aligned = (platformPos.y() == busPos.y() && platformPos.x() < busPos.x()); break;
            case Direction::Down:  aligned = (platformPos.x() == busPos.x() && platformPos.y() > busPos.y()); break;
            case Direction::Up:    aligned = (platformPos.x() == busPos.x() && platformPos.y() < busPos.y()); break;
        }

        if (aligned)
            return platform;
    }

    return nullptr;
}

Platform* Board::findTargetPlatform(int busId)
{
    // Reutiliza a versão const via const_cast seguro (este objeto não é const).
    return const_cast<Platform*>(
        static_cast<const Board*>(this)->findTargetPlatform(busId)
    );
}

// ─────────────────────────────────────────────────────────────────────────────
// Condição de vitória
// ─────────────────────────────────────────────────────────────────────────────

bool Board::allPassengersBoarded() const
{
    return std::all_of(
        passengers_.cbegin(), passengers_.cend(),
        [](const std::unique_ptr<Passenger>& p) { return p->isBoarded(); }
    );
}

bool Board::allBusesLeft() const
{
    return std::all_of(
        buses_.cbegin(), buses_.cend(),
        [](const std::unique_ptr<Bus>& b) { return b->hasLeft(); }
    );
}

bool Board::isLevelComplete() const
{
    return allPassengersBoarded() && allBusesLeft();
}

int Board::waitingPassengerCount() const
{
    return static_cast<int>(
        std::count_if(
            passengers_.cbegin(), passengers_.cend(),
            [](const std::unique_ptr<Passenger>& p) { return p->isWaiting(); }
        )
    );
}

int Board::activeBusCount() const
{
    return static_cast<int>(
        std::count_if(
            buses_.cbegin(), buses_.cend(),
            [](const std::unique_ptr<Bus>& b) { return b->isActive(); }
        )
    );
}

// ─────────────────────────────────────────────────────────────────────────────
// Reset
// ─────────────────────────────────────────────────────────────────────────────

void Board::reset()
{
    qDebug().noquote()
        << "Board::reset — a reiniciar tabuleiro" << rows_ << "x" << cols_;

    // Passo 1: delega reset a todas as entidades (posições e estados).
    // Usa std::as_const para evitar COW detach em QList<unique_ptr> (não copiável).
    for (const auto& busPtr : std::as_const(buses_))
        busPtr->reset();

    for (const auto& passengerPtr : std::as_const(passengers_))
        passengerPtr->reset();

    for (const auto& platformPtr : std::as_const(platforms_))
        platformPtr->reset();

    // Passo 2: reconstrói as grelhas a partir das posições iniciais
    // (que foram restauradas pelo reset() de cada entidade).
    rebuildGrids();

    qDebug().noquote()
        << "Board::reset — concluído.";
}

// ─────────────────────────────────────────────────────────────────────────────
// Utilitários
// ─────────────────────────────────────────────────────────────────────────────

QString Board::toString() const
{
    return QString("Board[%1x%2 buses=%3 passengers=%4 platforms=%5 complete=%6]")
        .arg(rows_)
        .arg(cols_)
        .arg(busCount())
        .arg(passengerCount())
        .arg(platformCount())
        .arg(isLevelComplete() ? "true" : "false");
}

QString Board::toAsciiGrid() const
{
    QString result;
    result.reserve((cols_ + 1) * rows_);

    for (int r = 0; r < rows_; ++r)
    {
        for (int c = 0; c < cols_; ++c)
        {
            result += occupancyChar(occupancyGrid_[r][c]);
        }
        result += '\n';
    }

    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// Auxiliares privados
// ─────────────────────────────────────────────────────────────────────────────

void Board::clearGrids()
{
    occupancyGrid_.resize(rows_);
    idGrid_.resize(rows_);

    for (int r = 0; r < rows_; ++r)
    {
        occupancyGrid_[r].fill(OccupancyType::None, cols_);
        idGrid_[r].fill(-1, cols_);
    }
}

void Board::rebuildGrids()
{
    // Limpa tudo antes de repopular para garantir consistência.
    clearGrids();

    // Plataformas primeiro (são estruturais e têm prioridade na grelha).
    for (const auto& platformPtr : std::as_const(platforms_))
    {
        const QPoint pos = platformPtr->position();

        // Aqui não usamos canOccupyCell porque, após reset, a grelha está
        // limpa e sabemos que as posições são válidas (foram validadas em addPlatform).
        Q_ASSERT(isValidPosition(pos));

        occupancyGrid_[pos.y()][pos.x()] = OccupancyType::Platform;
        idGrid_[pos.y()][pos.x()]        = platformPtr->id();
    }

    // Autocarros ativos (os que já saíram não ocupam células).
    for (const auto& busPtr : std::as_const(buses_))
    {
        if (!busPtr->isActive() || busPtr->hasLeft())
            continue;

        const QPoint pos = busPtr->position();
        Q_ASSERT(isValidPosition(pos));

        // Um autocarro pode estar sobre uma plataforma no momento do reset
        // se o estado foi guardado durante o embarque. Nesse caso, a
        // plataforma já está marcada e o Bus sobrescreve — comportamento
        // intencional (Bus tem prioridade visual sobre Platform).
        occupancyGrid_[pos.y()][pos.x()] = OccupancyType::BusEntity;
        idGrid_[pos.y()][pos.x()]        = busPtr->id();
    }

    // Passageiros em Waiting (os em Boarding/Boarded não ocupam células).
    for (const auto& passengerPtr : std::as_const(passengers_))
    {
        if (!passengerPtr->isWaiting())
            continue;

        const QPoint pos = passengerPtr->cellPosition();
        Q_ASSERT(isValidPosition(pos));
        Q_ASSERT(occupancyGrid_[pos.y()][pos.x()] == OccupancyType::None);

        occupancyGrid_[pos.y()][pos.x()] = OccupancyType::Passenger;
        idGrid_[pos.y()][pos.x()]        = passengerPtr->id();
    }
}

bool Board::canOccupyCell(QPoint pos, const char* caller) const
{
    if (!isValidPosition(pos))
    {
        qWarning().noquote()
            << "Board::" << caller
            << ": posição fora dos limites" << pos
            << ". Tabuleiro:" << rows_ << "x" << cols_;
        return false;
    }

    if (occupancyGrid_[pos.y()][pos.x()] != OccupancyType::None)
    {
        qWarning().noquote()
            << "Board::" << caller
            << ": célula" << pos << "já está ocupada."
            << "tipo=" << static_cast<int>(occupancyGrid_[pos.y()][pos.x()])
            << "id=" << idGrid_[pos.y()][pos.x()];
        return false;
    }

    return true;
}

template<typename T>
bool Board::isDuplicateId(const std::vector<std::unique_ptr<T>>& list, int id) const
{
    return std::any_of(
        list.cbegin(), list.cend(),
        [id](const std::unique_ptr<T>& item) { return item->id() == id; }
    );
}

template<typename T>
T* Board::findById(const std::vector<std::unique_ptr<T>>& list, int id)
{
    for (const auto& item : list)
    {
        if (item->id() == id)
            return item.get();
    }
    return nullptr;
}

char Board::occupancyChar(OccupancyType type) noexcept
{
    switch (type)
    {
        case OccupancyType::None:      return '.';
        case OccupancyType::BusEntity: return 'B';
        case OccupancyType::Passenger: return 'P';
        case OccupancyType::Platform:  return 'X';
    }
    Q_UNREACHABLE();
    return '?';
}

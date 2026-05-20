#include "MazeGenerator.h"
#include <stack>
#include <algorithm>
#include <chrono>
#include <queue>
#include <iostream>

MazeGenerator::MazeGenerator(int w, int h) : width(w), height(h), maze(h, std::vector<char>(w, '#')) {
    rng.seed(std::chrono::steady_clock::now().time_since_epoch().count());
    startX = startY = endX = endY = robotX = robotY = 0;
}

// provera da li je polje u unutrasnjosti
bool MazeGenerator::isValidCell(int x, int y) const {
    return x > 0 && y > 0 && x < width - 1 && y < height - 1;
}

void MazeGenerator::generate() {
    std::stack<std::pair<int, int>> st;

    // nasumican ulaz na gornjoj ivici
    startX = (rng() % (width - 2)) + 1;
    startY = 0;
    maze[startY][startX] = 'U';

    // robot je odmah ispod ulaza
    robotX = startX;
    robotY = startY + 1;
    maze[robotY][robotX] = 'R';

    // startna celija za DFS - koristimo robotX, robotY
    st.push({ robotX, robotY });

    // dx i dy predstavljaju 4 osnovna smera - gore, dole, levo, desno
    const int dx[4] = { 0, 0, -1, 1 };
    const int dy[4] = { -1, 1, 0, 0 };

    // dok ima celija na steku, pokusaj prosirenja puta iz aktuelne celije
    while (!st.empty()) {
        // uzimamo sa vrha steka
        int x = st.top().first;
        int y = st.top().second;

        // random redosled pravaca
        std::vector<int> dirs = { 0, 1, 2, 3 };
        std::shuffle(dirs.begin(), dirs.end(), rng);

        bool moved = false;
        for (int k = 0; k < 4; ++k) {
            int dir = dirs[k];

            // pomeramo se za dva koraka - prvo prazan prostor za prolaz, pa sledeća celija
            // da imamo zidove izmedju celija
            int nx = x + 2 * dx[dir];
            int ny = y + 2 * dy[dir];

            if (isValidCell(nx, ny) && maze[ny][nx] == '#') {
                // probijamo zid izmedju y + dy, x + dx - postaje prolaz
                maze[y + dy[dir]][x + dx[dir]] = '.';
                // nova celija takodje postaje prolaz
                maze[ny][nx] = '.';
                // stavljamo novu celiju na stek da nastavimo DFS iz nje
                st.push({ nx, ny });
                moved = true;
                break;
            }
        }
        if (!moved) 
            st.pop(); // ako nemamo gde, vracamo se - backtrack
    }

    // postavljanje izlaza na poslednjem redu - trazimo polje na pretposlednjem redu koje je prohodno
    endX = endY = -1;
    for (int x = 1; x < width - 1; ++x) {
        // ako je polje iznad poslednjeg reda prelaz, koristimo to mesto za izlaz
        if (maze[height - 2][x] == '.') {
            endX = x;
            endY = height - 1;
            maze[endY][endX] = 'I';
            break;
        }
    }

    // ako nije nadjen prikladan izlaz, backup slucaj - postavimo ga u sredinu
    if (endX == -1) {  // backup izlaz
        endX = width / 2;
        endY = height - 1;
        maze[height - 2][endX] = '.'; // probijamo polje iznad izlaza da bi put bio moguc
        maze[endY][endX] = 'I';
    }
}

// oznacavamo put od R do I i pamtimo koordinate puta
void MazeGenerator::highlightPath() {
    if (endX == -1 || endY == -1) // u slucaju da nema izlaza
        return;

    putPolja.clear(); // brisemo prethodni zapis puta

    // dist se koristi za BFS: -1 znaci neposeceno
    // parent cuva roditelja da bismo rekonstruisali put
    std::vector<std::vector<int>> dist(height, std::vector<int>(width, -1));
    std::vector<std::vector<std::pair<int, int>>> parent(height, std::vector<std::pair<int, int>>(width, { -1,-1 }));

    std::queue<std::pair<int, int>> q;
    q.push({ robotX, robotY });
    dist[robotY][robotX] = 0; // pocetna distanca

    const int dx[4] = { 0,0,-1,1 };
    const int dy[4] = { -1,1,0,0 };

    while (!q.empty()) {
        int x = q.front().first;
        int y = q.front().second;
        q.pop();

        if (x == endX && y == endY)  // pronasli smo izlaz, roditelji su popunjeni
            break;

        for (int d = 0; d < 4; ++d) {
            int nx = x + dx[d];
            int ny = y + dy[d];
            if (nx >= 0 && ny >= 0 && nx < width && ny < height) {
                // dozvoljavamo prolaz kroz sve osim zidova #
                if (maze[ny][nx] != '#' && dist[ny][nx] == -1) {
                    dist[ny][nx] = dist[y][x] + 1;
                    parent[ny][nx] = { x, y }; // cuvamo put
                    q.push({ nx, ny });
                }
            }
        }
    }

    // rekonstruisanje puta od izlaza nazad do robota koristeci parent matricu
    int x = endX, y = endY;
    while (!(x == robotX && y == robotY)) {
        if (maze[y][x] == '.') {
            putPolja.push_back({ x, y }); // pamti koordinate puta
            maze[y][x] = '*'; // plavi put
        }
        auto p = parent[y][x];
        x = p.first;
        y = p.second;
    }
}

// postavljanje predmeta P po oznacenom putu (putPolja) i postavljanje Minotaura M
void MazeGenerator::postaviPredmeteIMinotaura(int brojPredmeta) {
    std::uniform_int_distribution<int> distPut(0, putPolja.size() - 1);
    std::uniform_int_distribution<int> distX(1, width - 2);
    std::uniform_int_distribution<int> distY(1, height - 2);

    // postavljanje predmeta na putu
    int postavljeniPredmeti = 0;
    while (postavljeniPredmeti < brojPredmeta && !putPolja.empty()) {
        int idx = distPut(rng); // nasumicno biramo indeks polja puta
        int x = putPolja[idx].first;
        int y = putPolja[idx].second;

        if (maze[y][x] == '*') {
            maze[y][x] = 'P';
            postavljeniPredmeti++;
        }
    }

    // postavljanje Minotaura van puta
    std::vector<std::pair<int, int>> mogucaPolja;
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            char c = maze[y][x];
            if (c != '*' && c != 'R' && c != 'U' && c != 'I' && c != 'P') {
                int dx[4] = { 0,0,-1,1 };
                int dy[4] = { -1,1,0,0 };
                int slobodna = 0;
                for (int i = 0; i < 4; ++i) {
                    int nx = x + dx[i];
                    int ny = y + dy[i];
                    if (maze[ny][nx] == '.') // potreban je bar jedan susedni prolaz
                        slobodna++;
                }
                if (slobodna >= 1) 
                    mogucaPolja.push_back({ x, y });
            }
        }
    }

    // random izbor od mogucih opcija za Minotaura
    if (!mogucaPolja.empty()) {
        int idx = rng() % mogucaPolja.size();
        int mx = mogucaPolja[idx].first;
        int my = mogucaPolja[idx].second;
        maze[my][mx] = 'M';
    }
}

// prikaz lavirinta, moguca oba slucaja i sa vidljivom putanjom zbog preglednosti
void MazeGenerator::printMaze(bool withPath) const {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            char c = maze[y][x];

            if (!withPath && c == '*') {
                std::cout << '.'; // u prvom ispisu prikazujemo put kao .
            }
            else if (c == '*') {
                std::cout << "\033[34m*\033[0m"; // plavi put
            }
            else if (c == 'I' || c == 'U') {
                std::cout << "\033[35m" << c << "\033[0m"; // ljubicasto
            }
            else if (c == 'R') {
                std::cout << "\033[32m" << c << "\033[0m"; // zeleno
            }
            else if (c == 'P') {
                std::cout << "\033[33m" << c << "\033[0m"; // zuto
            }
            else if (c == 'M') {
                std::cout << "\033[31m" << c << "\033[0m"; // crveno
            }
            else {
                std::cout << c; // # ili .
            }
        }
        std::cout << "\n";
    }
}

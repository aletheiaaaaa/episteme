#include "chess/movegen.h"
#include "chess/perft.h"

#include <cstdint>
#include <array>
#include <iostream>
#include <chrono>
#include <iomanip>

using namespace valhalla;

int main(){
    Position position;
    position.fromStartPos();
    std::cout << position.toFEN() << std::endl;
    for (int i = 1; i <= 6; i++) {
        timePerft(position, i);
    }
    return 0;
}
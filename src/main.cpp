#include "APP.h"
#include "SuperCar.h"

void setup()
{
    APP.startTask();        // Web / APP 控制（如果你还需要）
    BalanceCar.startTask(); // 平衡车初始化
}

void loop()
{
    BalanceCar.running();   // 实时控制循环
}

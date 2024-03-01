//
// Created by 37496 on 2024/2/10.
//

#include "base/TimeStamp.h"
#include <vector>
#include <stdio.h>

using Tiny_muduo::TimeStamp;

void passByConstReference(const Tiny_muduo::TimeStamp& x)
{
    printf("%s\n", x.toFormatString().c_str());
}

void passByValue(Tiny_muduo::TimeStamp x)
{
    printf("%s\n", x.toFormatString().c_str());
}

void benchmark()
{
    const int kNumber = 1000*1000;

    std::vector<Tiny_muduo::TimeStamp> stamps;
    stamps.reserve(kNumber);
    for (int i = 0; i < kNumber; ++i)
    {
        stamps.push_back(Tiny_muduo::TimeStamp::now());
    }
    printf("%s\n", stamps.front().toFormatString().c_str());
    printf("%s\n", stamps.back().toFormatString().c_str());
    printf("%f\n", timeDifference(stamps.back(), stamps.front()));

    int increments[100] = { 0 };
    int64_t start = stamps.front().microSecond();
    for (int i = 1; i < kNumber; ++i)
    {
        int64_t next = stamps[i].microSecond();
        int64_t inc = next - start;
        start = next;
        if (inc < 0)
        {
            printf("reverse!\n");
        }
        else if (inc < 100)
        {
            ++increments[inc];
        }
        else
        {
            printf("big gap %d\n", static_cast<int>(inc));
        }
    }

    for (int i = 0; i < 100; ++i)
    {
        printf("%2d: %d\n", i, increments[i]);
    }
}

int main()
{
    Tiny_muduo::TimeStamp now(Tiny_muduo::TimeStamp::now());
    printf("%s\n", now.toFormatString().c_str());
    passByValue(now);
    passByConstReference(now);
    benchmark();
}

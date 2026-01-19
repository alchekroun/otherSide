#include "app/App.h"
#include <iostream>

int main(int argc, char **argv)
{
    otherside::Application *a = new otherside::Application();

    a->start();
    a->stop();

    return 0;
}
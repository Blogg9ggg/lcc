//
// Created by blog on 2022/4/4.
//

#include <stdio.h>

int add(int a, int b) {
    return a + b;
}
int main() {
    int ans;
    ans = add(1, 2);
    printf("1 + 2 = %d\n", ans);

    return 0;
}
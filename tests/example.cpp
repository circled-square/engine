int main() {
    const int a = 2;
    const int b = 2;

    const int result = a+b;
    const int expected = 4;

    if(result != expected)
        return -1;

    return 0;
}

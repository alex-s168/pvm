CLASS(Test) {
    HEADER

    MUT_FIELD(T_ARR, name)
        OFFSET(offsetof(struct cTest, fname))

    MUT_FIELD(T_ARR, email)
        OFFSET(offsetof(struct cTest, femail))
};
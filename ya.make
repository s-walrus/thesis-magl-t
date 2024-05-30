LIBRARY()

PEERDIR(
    src
)

END()

RECURSE(
    src
)

RECURSE_FOR_TESTS(
    ut
    benchmark
)

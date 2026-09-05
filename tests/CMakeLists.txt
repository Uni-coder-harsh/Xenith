include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

# Unit tests
add_executable(test_vector_ops unit/numerics/test_vector_ops.cpp)
target_link_libraries(test_vector_ops PRIVATE xenith_lib)
add_test(NAME unit_vector_ops COMMAND test_vector_ops)

add_executable(test_sparse_matrix unit/numerics/test_sparse_matrix.cpp)
target_link_libraries(test_sparse_matrix PRIVATE xenith_lib)
add_test(NAME unit_sparse_matrix COMMAND test_sparse_matrix)

add_executable(test_canonical_model unit/model/test_canonical_model.cpp)
target_link_libraries(test_canonical_model PRIVATE xenith_lib)
add_test(NAME unit_canonical_model COMMAND test_canonical_model)

add_executable(test_model_validator unit/model/test_model_validator.cpp)
target_link_libraries(test_model_validator PRIVATE xenith_lib)
add_test(NAME unit_model_validator COMMAND test_model_validator)

# Integration tests
add_executable(test_model_numerics_pipeline integration/test_model_numerics_pipeline.cpp)
target_link_libraries(test_model_numerics_pipeline PRIVATE xenith_lib)
add_test(NAME integration_model_numerics COMMAND test_model_numerics_pipeline)

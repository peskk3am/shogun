library("sg")

size_cache <- 10

fm_train_real <- t(as.matrix(read.table('../data/fm_train_real.dat')))
fm_test_real <- t(as.matrix(read.table('../data/fm_test_real.dat')))

# GaussianShift
print('GaussianShift')

width <- 1.8
max_shift <- 2
shift_step <- 1

dump <- sg('set_kernel', 'GAUSSIANSHIFT', 'REAL', size_cache, width, max_shift, shift_step)

dump <- sg('set_features', 'TRAIN', fm_train_real)
km <- sg('get_kernel_matrix', 'TRAIN')

dump <- sg('set_features', 'TEST', fm_test_real)
km <- sg('get_kernel_matrix', 'TEST')

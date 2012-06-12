/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 2012 Heiko Strathmann
 */

#include <shogun/base/init.h>
#include <shogun/statistics/QuadraticTimeMMD.h>
#include <shogun/kernel/GaussianKernel.h>
#include <shogun/features/DenseFeatures.h>
#include <shogun/mathematics/Statistics.h>

using namespace shogun;


void create_mean_data(SGMatrix<float64_t> target, float64_t difference)
{
	/* create data matrix for P and Q. P is a standard normal, Q is the same but
	 * has a mean difference in one dimension */
	for (index_t i=0; i<target.num_rows; ++i)
	{
		for (index_t j=0; j<target.num_cols/2; ++j)
			target(i,j)=CMath::randn_double();

		/* add mean difference in first dimension of second half of data */
		for (index_t j=target.num_cols/2; j<target.num_cols; ++j)
				target(i,j)=CMath::randn_double() + (i==0 ? difference : 0);
	}
}

/** tests the quadratic mmd statistic for a single data case and ensures
 * equality with matlab implementation */
void test_quadratic_mmd_fixed()
{
	index_t n=2;
	index_t d=3;
	float64_t sigma=2;
	float64_t sq_sigma_twice=sigma*sigma*2;
	SGMatrix<float64_t> data(d,2*n);
	for (index_t i=0; i<2*d*n; ++i)
		data.matrix[i]=i;

	CDenseFeatures<float64_t>* features=new CDenseFeatures<float64_t>(data);
	CGaussianKernel* kernel=new CGaussianKernel(10, sq_sigma_twice);
	kernel->init(features, features);

	CQuadraticTimeMMD* mmd=new CQuadraticTimeMMD(kernel, features, n);

	float64_t difference=CMath::abs(mmd->compute_statistic()-0.051325806508381);
	ASSERT(difference<=10E-16);

	SG_UNREF(mmd);
}

void test_quadratic_mmd_bootstrap()
{
	index_t dimension=3;
	index_t m=300;
	float64_t difference=0.5;
	float64_t sigma=2;

	SGMatrix<float64_t> data(dimension, 2*m);
	create_mean_data(data, difference);
	CDenseFeatures<float64_t>* features=new CDenseFeatures<float64_t>(data);

	/* shoguns kernel width is different */
	CGaussianKernel* kernel=new CGaussianKernel(100, sigma*sigma*2);
	CQuadraticTimeMMD* mmd=new CQuadraticTimeMMD(kernel, features, m);
	mmd->set_bootstrap_iterations(100);
	SGVector<float64_t> null_samples=mmd->bootstrap_null();

	null_samples.display_vector();
	SG_SPRINT("mean %f, var %f\n", CStatistics::mean(null_samples),
			CStatistics::variance(null_samples));

	SG_UNREF(mmd);
}

void test_spectrum(CQuadraticTimeMMD* mmd)
{
	mmd->set_num_samples_sepctrum(250);
	mmd->set_num_eigenvalues_spectrum(2);
	mmd->set_p_value_method(MMD2_SPECTRUM);
	SG_SPRINT("spectrum: %f\n", mmd->compute_p_value(2));
}

void test_gamma(CQuadraticTimeMMD* mmd)
{
	mmd->set_p_value_method(MMD2_GAMMA);
	SG_SPRINT("gamma: %f\n", mmd->compute_p_value(2));
}

/** tests the quadratic mmd statistic for a random data case (fixed distribution
 * and ensures equality with matlab implementation */
void test_quadratic_mmd_random()
{
	index_t dimension=3;
	index_t m=300;
	float64_t difference=0.5;
	float64_t sigma=2;

	index_t num_runs=100;
	SGVector<float64_t> mmds(num_runs);

	SGMatrix<float64_t> data(dimension, 2*m);

	CDenseFeatures<float64_t>* features=new CDenseFeatures<float64_t>(data);

	/* shoguns kernel width is different */
	CGaussianKernel* kernel=new CGaussianKernel(100, sigma*sigma*2);
	CQuadraticTimeMMD* mmd=new CQuadraticTimeMMD(kernel, features, m);
	for (index_t i=0; i<num_runs; ++i)
	{
		create_mean_data(data, difference);
		kernel->init(features, features);
		mmds[i]=mmd->compute_statistic();
	}

	/* MATLAB 95% mean confidence interval 0.007495841715582 0.037960088792417 */
	float64_t mean=CStatistics::mean(mmds);
	ASSERT((mean>0.007495841715582) && (mean<0.037960088792417));

	/* MATLAB variance is 5.800439687240292e-05 quite stable */
	float64_t variance=CStatistics::variance(mmds);
	ASSERT(CMath::abs(variance-5.800439687240292e-05)<10E-5);
	SG_UNREF(mmd);
}

int main(int argc, char** argv)
{
	init_shogun_with_defaults();

	test_quadratic_mmd_fixed();
	test_quadratic_mmd_random();
//	test_quadratic_mmd_bootstrap();

	exit_shogun();
	return 0;
}


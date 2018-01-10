#include "ocl_boiler.h"

#define ROWS 5
#define COLS 10

int error(const char *msg) {
	fprintf(stderr, "%s\n", msg);
	exit(1);
}

int lws_cli;
int vec_kernel;

size_t preferred_wg_init;
cl_event init(cl_command_queue que,
	cl_kernel init_k,
	cl_mem d_mat, cl_int rows, cl_int cols)
{
	size_t lws[] = { lws_cli };
	/* se il local work size è stato specificato, arrotondiamo il
	 * global work size al multiplo successivo del lws, altrimenti,
	 * lo arrotondiamo al multiplo successivo della base preferita
	 * dalla piattaforma */
	size_t gws[] = {
 		round_mul_up(cols, preferred_wg_init),
 		round_mul_up(rows, preferred_wg_init),
  };
	cl_event init_evt;
	cl_int err;

	err = clSetKernelArg(init_k, 0, sizeof(d_mat), &d_mat);
	ocl_check(err, "set init arg 0");
	err = clSetKernelArg(init_k, 1, sizeof(rows), &rows);
	ocl_check(err, "set init arg 1");
	err = clSetKernelArg(init_k, 2, sizeof(cols), &cols);
	ocl_check(err, "set init arg 2");

	err = clEnqueueNDRangeKernel(que, init_k,
		2, NULL, gws, (lws_cli ? lws : NULL), /* griglia di lancio */
		0, NULL, /* waiting list */
		&init_evt);
	ocl_check(err, "enqueue kernel init");

	return init_evt;
}

size_t preferred_wg_generation;
cl_event generation(cl_command_queue que,
	cl_kernel generation_k,
	cl_mem d_dst, cl_mem d_src,
	cl_int s_rows, cl_int s_cols,
	cl_event init_evt)
{
	size_t gws[] = {
		round_mul_up(s_cols, preferred_wg_generation),
		round_mul_up(s_rows, preferred_wg_generation),
       	};
	cl_event generation_evt;
	cl_int err;

	err = clSetKernelArg(generation_k, 0,
		sizeof(d_dst), &d_dst);
	ocl_check(err, "set generation arg 0");
	err = clSetKernelArg(generation_k, 1, sizeof(d_src), &d_src);
	ocl_check(err, "set generation arg 1");
	err = clSetKernelArg(generation_k, 2, sizeof(s_rows), &s_rows);
	ocl_check(err, "set generation arg 2");
	err = clSetKernelArg(generation_k, 3, sizeof(s_cols), &s_cols);
	ocl_check(err, "set generation arg 3");

	cl_event wait_list[] = { init_evt };
	err = clEnqueueNDRangeKernel(que, generation_k,
		2, NULL, gws, NULL, /* griglia di lancio */
		1, wait_list, /* waiting list */
		&generation_evt);
	ocl_check(err, "enqueue kernel generation");

	return generation_evt;
}

int main(int argc, char *argv[])
{
	int rows = ROWS;
	int cols = COLS;
	/*
	if (argc < 3)
		error("inserire numero di elementi");

	int rows = atoi(argv[1]);
	int cols = atoi(argv[2]);
	if (rows <= 0 || cols <= 0)
		error("il numero di righe/colonne deve essere positivo");

	*/
	const size_t memsize = sizeof(int)*rows*cols;

	/* Hic sunt leones */

	cl_platform_id p = select_platform();
	cl_device_id d = select_device(p);
	cl_context ctx = create_context(p, d);
	cl_command_queue que = create_queue(ctx, d);
	cl_program prog = create_program("kernels.ocl", ctx, d);

	cl_int err;

	/* Extract kernels */

	/* per i kernel vettoriali, nels deve essere multiplo
	 * della larghezza vettoriale del kernel */
	 cl_kernel init_k = clCreateKernel(prog, "init", &err);
 	ocl_check(err, "create kernel init");
	cl_kernel generation_k = clCreateKernel(prog, "generation", &err);
	ocl_check(err, "create kernel generation");

	err = clGetKernelWorkGroupInfo(init_k, d,
		CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
		sizeof(preferred_wg_init), &preferred_wg_init, NULL);
	err = clGetKernelWorkGroupInfo(generation_k, d,
		CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
		sizeof(preferred_wg_generation), &preferred_wg_generation, NULL);

	cl_mem mat = clCreateBuffer(ctx, CL_MEM_READ_WRITE,
			memsize, NULL, &err);
	ocl_check(err, "create buffer");
	cl_mem d_dst = clCreateBuffer(ctx, CL_MEM_READ_WRITE,
		memsize, NULL, &err);
	ocl_check(err, "create buffer dst");

	cl_event init_evt = init(que, init_k, mat, rows, cols);

	cl_event copy_evt;

	int *dst = clEnqueueMapBuffer(que, mat, CL_TRUE,
			CL_MAP_READ, 0, memsize,
			1, &init_evt, &copy_evt, &err);
	ocl_check(err, "read buffer");

	int numb = 0;

	for(int i = 0; i < rows*cols; i++){
			if(numb == 9){
				numb = 0;
				printf("%d\n", dst[i]);
			}
			else{
				printf("%d", dst[i]);
				numb++;
			}
	}
	printf("\n");

	cl_event generation_evt = generation(que, generation_k,
		d_dst, mat, rows, cols, init_evt);

		dst = clEnqueueMapBuffer(que, d_dst, CL_TRUE,
				CL_MAP_READ, 0, memsize,
				1, &generation_evt, &copy_evt, &err);
		ocl_check(err, "read buffer");

		numb = 0;

		for(int i = 0; i < rows*cols; i++){
				if(numb == 9){
					numb = 0;
					printf("%d\n", dst[i]);
				}
				else{
					printf("%d", dst[i]);
					numb++;
				}
		}
		printf("\n");

	printf("init time:\t%gms\t%gGB/s\n", runtime_ms(init_evt),
		(2.0*memsize)/runtime_ns(init_evt));
	printf("generation time:\t%gms\t%gGB/s\n", runtime_ms(generation_evt),
		(2.0*memsize)/runtime_ns(generation_evt));
	printf("copy time:\t%gms\t%gGB/s\n", runtime_ms(copy_evt),
		(2.0*memsize)/runtime_ns(copy_evt));

	return 0;
}

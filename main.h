#define ROWS_DIEHARD 5
#define COLS_DIEHARD 10

#define ROWS_GOSPER 11
#define COLS_GOSPER 38

#define GENER 130

int error(const char *msg) {
	fprintf(stderr, "%s\n", msg);
	exit(1);
}

void print(int rows, int cols, cl_mem mat, cl_event ev, cl_command_queue que, size_t memsize, cl_int err){
  cl_event copy_evt;

	int *dst = clEnqueueMapBuffer(que, mat, CL_TRUE,
			CL_MAP_READ, 0, memsize,
			1, &ev, &copy_evt, &err);
	ocl_check(err, "read buffer");

	int numb = 0;

	for(int i = 0; i < rows*cols; i++){
			if(numb == cols - 1){
				numb = 0;
				printf("%d\n", dst[i]);
			}
			else{
				printf("%d", dst[i]);
				numb++;
			}
	}
	printf("\n");
  printf("copy time:\t%gms\t%gGB/s\n", runtime_ms(copy_evt),
		(2.0*memsize)/runtime_ns(copy_evt));
}

int lws_cli;

size_t preferred_wg_init;
cl_event init(cl_command_queue que,
	cl_kernel init_k,
	cl_mem d_mat, cl_int rows, cl_int cols, cl_char init)
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
	err = clSetKernelArg(init_k, 3, sizeof(init), &init);
	ocl_check(err, "set init arg 3");

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

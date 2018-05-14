int lws_cli;

size_t preferred_wg_init;
cl_event init(cl_command_queue que,
	cl_kernel init_k,
	cl_mem d_mat, cl_int rows, cl_int cols, cl_char init)
{
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
		2, NULL, gws, NULL, /* griglia di lancio */
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
	size_t lws[] = { lws_cli, lws_cli };
	size_t gws[] = {
 		round_mul_up(s_cols, lws_cli ? lws[0] : preferred_wg_generation),
 		round_mul_up(s_rows, lws_cli ? lws[1] : preferred_wg_generation),
  };
	cl_event generation_evt;
	cl_int err;

	err = clSetKernelArg(generation_k, 0,
		sizeof(d_dst), &d_dst);
	ocl_check(err, "set generation arg 0");
	err = clSetKernelArg(generation_k, 1, sizeof(d_src), &d_src);
	ocl_check(err, "set generation arg 1");
	err = clSetKernelArg(generation_k, 2, (lws[0]+2)*sizeof(cl_int), NULL);
	ocl_check(err, "set generation arg 2");
	err = clSetKernelArg(generation_k, 3, sizeof(s_rows), &s_rows);
	ocl_check(err, "set generation arg 3");
	err = clSetKernelArg(generation_k, 4, sizeof(s_cols), &s_cols);
	ocl_check(err, "set generation arg 4");

	cl_event wait_list[] = { init_evt };
	err = clEnqueueNDRangeKernel(que, generation_k,
		2, NULL, gws, lws, /* griglia di lancio */
		1, wait_list, /* waiting list */
		&generation_evt);
	ocl_check(err, "enqueue kernel generation");

	return generation_evt;
}

size_t preferred_wg_expand;
cl_event expand(cl_command_queue que,
	cl_kernel expand_k,
	cl_mem d_dst, cl_mem d_src, cl_mem d_sides,
	cl_int s_rows, cl_int s_cols,
	cl_event init_evt)
{
	size_t gws[] = {
		round_mul_up(s_cols, preferred_wg_expand),
		round_mul_up(s_rows, preferred_wg_expand),
       	};
	cl_event expand_evt;
	cl_int err;

	err = clSetKernelArg(expand_k, 0,
		sizeof(d_dst), &d_dst);
	ocl_check(err, "set expand arg 0");
	err = clSetKernelArg(expand_k, 1, sizeof(d_src), &d_src);
	ocl_check(err, "set expand arg 1");
	err = clSetKernelArg(expand_k, 2, sizeof(d_sides), &d_sides);
	ocl_check(err, "set expand arg 2");
	err = clSetKernelArg(expand_k, 3, sizeof(s_rows), &s_rows);
	ocl_check(err, "set expand arg 3");
	err = clSetKernelArg(expand_k, 4, sizeof(s_cols), &s_cols);
	ocl_check(err, "set expand arg 4");

	cl_event wait_list[] = { init_evt };
	err = clEnqueueNDRangeKernel(que, expand_k,
		2, NULL, gws, NULL, /* griglia di lancio */
		1, wait_list, /* waiting list */
		&expand_evt);
	ocl_check(err, "enqueue kernel expand");

	return expand_evt;
}

size_t preferred_wg_where_expand;
cl_event where_expand(cl_command_queue que,
	cl_kernel where_expand_k,
	cl_mem d_src, cl_mem d_sides,
	cl_int s_rows, cl_int s_cols,
	cl_event generation_evt)
{
	size_t gws[] = {
		round_mul_up(s_cols, preferred_wg_where_expand),
		round_mul_up(s_rows, preferred_wg_where_expand),
       	};
	cl_event where_expand_evt;
	cl_int err;

	err = clSetKernelArg(where_expand_k, 0,
		sizeof(d_src), &d_src);
	ocl_check(err, "set where_expand arg 0");
	err = clSetKernelArg(where_expand_k, 1, sizeof(d_sides), &d_sides);
	ocl_check(err, "set where_expand arg 1");
	err = clSetKernelArg(where_expand_k, 2, sizeof(s_rows), &s_rows);
	ocl_check(err, "set where_expand arg 2");
	err = clSetKernelArg(where_expand_k, 3, sizeof(s_cols), &s_cols);
	ocl_check(err, "set where_expand arg 3");

	cl_event wait_list[] = { generation_evt };
	err = clEnqueueNDRangeKernel(que, where_expand_k,
		2, NULL, gws, NULL, /* griglia di lancio */
		1, wait_list, /* waiting list */
		&where_expand_evt);
	ocl_check(err, "enqueue kernel where_expand");

	return where_expand_evt;
}

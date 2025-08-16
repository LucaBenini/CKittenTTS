#include "kittentts.h"
static void kt_usage(const char* prog_name);
static int check_file(const char* filename);
int kt_create(kt_params** kp)
{
	*kp = calloc(1, sizeof(kt_params));
	if (!*kp)
	{
		return -1;
	}
	return 0;
}
int kt_load(kt_params* kp, int argc, char* argv[])
{
	if (!kp)
		return -1;

	if (argc < 6) {
		kt_usage(argv[0]);
		return -1;
	}

	// Assign parameters in order
	kp->init.model_path = argv[1];
	kp->init.voice_path = argv[2];
	kp->run.output_file = argv[3];
	kp->run.message = argv[4];
	kp->run.speed = (float)atof(argv[5]);

	return 0; // success
}

int kt_validate(kt_params* kp)
{
	if (!kp)
		return -1;
	if (check_file(kp->init.model_path))
	{
		fprintf(stderr, "Unable to read model: %s\n", kp->init.model_path ? kp->init.model_path: "(null)");
		return -1;
	}
	if (check_file(kp->init.voice_path))
	{
		fprintf(stderr, "Unable to read voice: %s\n", kp->init.voice_path ? kp->init.voice_path : "(null)");
		return -1;
	}
	if (!kp->run.output_file)
	{
		fprintf(stderr, "Missing output\n");
		return -1;
	}
	if (!kp->run.message)
	{
		fprintf(stderr, "Missing message\n");
		return -1;
	}
	if (kp->run.speed <= 0.1 || kp->run.speed > 5.0)
	{
		fprintf(stderr, "Speed is %f must be between 0.1 and 5.0\n", kp->run.speed);
		return -1;
	}
	return 0;
}
int kt_destroy(kt_params* kp)
{
	if (kp)
	{
		if (kp->run.input_ids)
		{
			free(kp->run.input_ids);
			kp->run.input_ids = 0;
			kp->run.input_ids_len = 0;
		}
		if (kp->run.output)
		{
			free(kp->run.output);
			kp->run.output = 0;
		}
		free(kp);
	}
	return 0;
}
static int check_file(const char* filename)
{
	if (!filename)
	{
		return -1;
	}
	if (!os_file_exists(filename))
		return -1;
	return 0;
}
static void kt_usage(const char* prog_name)
{
	fprintf(stderr, "Usage: %s <model_path> <voice_path> <message> <speed>\n", prog_name);
	fprintf(stderr, "  model_path : Path to the model file\n");
	fprintf(stderr, "  voice_path : Path to the voice file\n");
	fprintf(stderr, "  output     : Wav file to create\n");
	fprintf(stderr, "  message    : Text to synthesize\n");
	fprintf(stderr, "  speed      : Speech speed (float)\n");
}

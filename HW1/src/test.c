#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/hmm.h"

#define NUMBER_OF_MODEL 5
#define MAX_NUMBER_OF_OBSERVATION_SEQUENCE 2501
#define MAX_LENGTH_OF_OBSERVATION_SEQUENCE 51
#define MAX_LENGTH_OF_MODEL_NAME 512

struct Answer {
	int index;
	double likelihood;
};

typedef struct Answer Answer;

Answer Viterbi_algorithm(HMM hmm[NUMBER_OF_MODEL] , char *observation_sequence , int length_of_observation_sequence) {
	Answer output = {.index = 0, .likelihood = 0};

	for (int i = 0; i < NUMBER_OF_MODEL; i++) {
		double delta[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE] = {{0}};

        for (int j = 0; j < hmm[i].state_num; j++) {
			delta[0][j] = hmm[i].initial[j] * hmm[i].observation[observation_sequence[0] - 'A'][j];
        }

        for (int j = 1; j < length_of_observation_sequence; j++) {
			for (int k = 0; k < hmm[i].state_num; k++) {
				double max_likelihood = 0;
                for (int l = 0; l < hmm[i].state_num; l++) {
                    if (delta[j - 1][l] * hmm[i].transition[l][k] > max_likelihood) {
						max_likelihood = delta[j - 1][l] * hmm[i].transition[l][k];
                    }
                }

				delta[j][k] = max_likelihood * hmm[i].observation[observation_sequence[j] - 'A'][k];
			}
        }

		double max_likelihood = 0;
        for (int j = 0; j < hmm[i].state_num; j++) {
            if (delta[length_of_observation_sequence - 1][j] > max_likelihood) {
				max_likelihood = delta[length_of_observation_sequence - 1][j];
            }
        }

		if (max_likelihood > output.likelihood) {
			output.index = i;
			output.likelihood = max_likelihood;
		}
	}

	return output;
}

void testHMM(HMM hmm[NUMBER_OF_MODEL], char *test_data , char *result_file, char *answer_file) {
	int number_of_observation_sequence = 0, number_of_label_sequence = 0;
	char observation_sequences[MAX_NUMBER_OF_OBSERVATION_SEQUENCE][MAX_LENGTH_OF_OBSERVATION_SEQUENCE] = {{0}};
	char label_sequences[MAX_NUMBER_OF_OBSERVATION_SEQUENCE][MAX_LENGTH_OF_MODEL_NAME] = {{0}};

	FILE *testing_data = open_or_die(test_data , "r");
    while (fscanf(testing_data , "%s" , observation_sequences[number_of_observation_sequence]) != EOF){
		number_of_observation_sequence++;
    }
	int length_of_observation_sequence = strlen(observation_sequences[0]);
	fclose(testing_data);


    int length_of_label_sequence = 0;
    if (answer_file != NULL){
        FILE *label_file = open_or_die(answer_file , "r");
        while (fscanf(label_file , "%s" , label_sequences[number_of_label_sequence]) != EOF){
            number_of_observation_sequence++;
        }
        int length_of_label_sequence = strlen(label_sequences[0]);
        fclose(label_file);
    }

    double accuracy = 0;
	FILE *dump_file = open_or_die(result_file , "w");
	for (int i = 0 ; i < number_of_observation_sequence ; i++) {
		Answer output = Viterbi_algorithm(hmm , observation_sequences[i] , length_of_observation_sequence);
		fprintf(dump_file , "%s %e\n" , hmm[output.index].model_name , output.likelihood);
        if (answer_file != NULL){
            if(output.index == atoi(label_sequences[length_of_label_sequence-5])){
                accuracy++;
            }
        }
	}

    if (answer_file != NULL){
        accuracy /= number_of_observation_sequence;
    }
    printf("%f", accuracy * 100);

	fclose(dump_file);

	return;
}

int main(int argc , char **argv) {

    char *list_file = argv[1];
    char *test_data = argv[2];
    char *result_file = argv[3];
    char *answer_file = NULL;
    if(argc > 4){
        answer_file = argv[4];
    }

	HMM hmm[NUMBER_OF_MODEL];
	load_models(list_file, hmm, NUMBER_OF_MODEL);
	testHMM(hmm , test_data, result_file, answer_file);
	return 0;
}

/*


#define MAX_LENGTH_OF_MODEL_NAME 256

int main(int argc , char **argv)
{
	FILE *result = fopen(argv[1] , "r");
	FILE *answer = fopen(argv[2] , "r");
	FILE *dump_file = fopen(argv[3] , "w");

	double likelihood;
	char model_1[MAX_LENGTH_OF_MODEL_NAME] = {0} , model_2[MAX_LENGTH_OF_MODEL_NAME] = {0};
	int count = 0 , all = 0;

	while (fscanf(result , "%s %lf" , model_1 , &likelihood) != EOF && fscanf(answer , "%s" , model_2) != EOF)
	{
		all++;
		if (strcmp(model_1 , model_2) == 0)
			count++;
	}

	fprintf(dump_file , "%f\n" , (double)count / (double)all);

	fclose(result);
	fclose(answer);
	fclose(dump_file);
	return 0;
}

 
int main(int argc , char **argv) {
	HMM hmms[5];
	load_models( "modellist.txt", hmms, 5);
	dump_models( hmms, 5);
    // Parsing parameters
    char *init_file = argv[1];
    char *train_data = argv[3];
    char *result_file = argv[4];

	HMM hmm_initial;
	loadHMM( &hmm_initial, "../model_init.txt" );
	dumpHMM( stderr, &hmm_initial );

	printf("log(0.5) = %f\n", log(1.5) );

	return 0;
}
*/

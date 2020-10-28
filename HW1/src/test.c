#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/hmm.h"

#define NUMBER_OF_MODELS 5
#define MAX_NUMBER_OF_OBSERVATION_SEQUENCES 2501
#define MAX_LENGTH_OF_OBSERVATION_SEQUENCE 51
#define MAX_LENGTH_OF_MODEL_NAME 512

struct Answer {
	int index;
	double likelihood;
};

typedef struct Answer Answer;

Answer Viterbi_algorithm(HMM hmm[NUMBER_OF_MODELS], char *observation_sequences, int length_of_observation_sequence) {
	Answer output = {.index = 0, .likelihood = 0};

	for (int i = 0; i < NUMBER_OF_MODELS; i++) {
		double delta[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE] = {{0}};

        for (int j = 0; j < hmm[i].state_num; j++) {
			delta[0][j] = hmm[i].initial[j] * hmm[i].observation[observation_sequences[0] - 'A'][j];
        }

        for (int j = 1; j < length_of_observation_sequence; j++) {
			for (int k = 0; k < hmm[i].state_num; k++) {
				double max_likelihood = 0;
                for (int l = 0; l < hmm[i].state_num; l++) {
                    if (delta[j - 1][l] * hmm[i].transition[l][k] > max_likelihood) {
						max_likelihood = delta[j - 1][l] * hmm[i].transition[l][k];
                    }
                }

				delta[j][k] = max_likelihood * hmm[i].observation[observation_sequences[j] - 'A'][k];
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

void testHMM(HMM hmm[NUMBER_OF_MODELS], char *test_data, char *result_file, char *answer_file) {
	char observation_sequences[MAX_NUMBER_OF_OBSERVATION_SEQUENCES][MAX_LENGTH_OF_OBSERVATION_SEQUENCE] = {{0}};
	char label_sequences[MAX_NUMBER_OF_OBSERVATION_SEQUENCES][MAX_LENGTH_OF_MODEL_NAME] = {{0}};

	int number_of_observation_sequences = 0;
	FILE *testing_data = open_or_die(test_data, "r");
    while (fscanf(testing_data, "%s", observation_sequences[number_of_observation_sequences]) != EOF) {
		number_of_observation_sequences++;
    }
	int length_of_observation_sequence = strlen(observation_sequences[0]);
	fclose(testing_data);


	int number_of_label_sequence = 0;
    if (answer_file != NULL){
        FILE *label_file = open_or_die(answer_file, "r");
        while (fscanf(label_file, "%s", label_sequences[number_of_label_sequence]) != EOF){
            number_of_label_sequence++;
        }
        fclose(label_file);
    }
    int length_of_label_sequence = strlen(label_sequences[0]);

    double accuracy = 0;
	FILE *dump_file = open_or_die(result_file, "w");
	for (int i = 0; i < number_of_observation_sequences; i++) {
		Answer output = Viterbi_algorithm(hmm, observation_sequences[i], length_of_observation_sequence);
		fprintf(dump_file, "%s %e\n", hmm[output.index].model_name, output.likelihood);
        if (answer_file != NULL){
            if(output.index == label_sequences[i][length_of_label_sequence-5] - '1'){
                accuracy++;
            }
        }
	}
	fclose(dump_file);

    if (answer_file != NULL){
        accuracy /= number_of_observation_sequences;
    }
    printf("Testing accuracy: %.2f %%", accuracy * 100);

	return;
}

int main(int argc, char **argv) {
    // Parsing parameters
    char *list_file = argv[1];
    char *test_data = argv[2];
    char *result_file = argv[3];
    char *answer_file = NULL;
    if(argc > 4){
        answer_file = argv[4];
    }

	HMM hmm[NUMBER_OF_MODELS];
	load_models(list_file, hmm, NUMBER_OF_MODELS);
	testHMM(hmm, test_data, result_file, answer_file);
	return 0;
}

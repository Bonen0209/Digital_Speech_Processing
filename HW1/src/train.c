#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/hmm.h"

#define MAX_NUMBER_OF_OBSERVATION_SEQUENCES 10001
#define MAX_LENGTH_OF_OBSERVATION_SEQUENCE 51

void forward_algorithm(HMM *hmm, char *observation_sequence, int length_of_observation_sequence, double alphas[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE]) {
    // Initial the alphas with given values
	for (int i = 0; i < hmm->state_num; i++) {
		alphas[0][i] = hmm->initial[i] * hmm->observation[observation_sequence[0] - 'A'][i];
    }
        
    //Acumulate each alpha with the equation inside the slides
	for (int i = 1; i < length_of_observation_sequence; i++) {
		for (int j = 0; j < hmm->state_num; j++) {
			for (int k = 0; k < hmm->state_num; k++) {
				alphas[i][j] += alphas[i - 1][k] * hmm->transition[k][j] * hmm->observation[observation_sequence[i] - 'A'][j];
            }
        }
    }

	return;
}

void backward_algorithm(HMM *hmm , char *observation_sequence , int length_of_observation_sequence, double betas[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE]) {
    // Initial the betas with given values 
    for (int i = 0; i < hmm->state_num; i++) {
		betas[length_of_observation_sequence - 1][i] = 1;
    }

    //Acumulate each beta with the equation inside the slides
    for (int i = length_of_observation_sequence - 2; i > -1; i--) {
		for (int j = 0; j < hmm->state_num; j++) {
            for (int k = 0; k < hmm->state_num; k++) {
				betas[i][j] += hmm->transition[j][k] * hmm->observation[observation_sequence[i + 1] - 'A'][k] * betas[i + 1][k];
            }
		}
    }

	return;
}

void calculate_gamma(HMM *hmm, char *observation_sequence, int length_of_observation_sequence, int number_of_observation_sequence, \
        double alphas[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE], \
        double betas[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE], \
        double gammas[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE], \
        double total_gammas_with_observation[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE][MAX_OBSERV]) {
	for (int i = 0; i < length_of_observation_sequence; i++) {
        //Calculate denominator
		double sum = 0;
        for (int j = 0; j < hmm->state_num; j++) {
			sum += alphas[i][j] * betas[i][j];
        }

        //Calculate numerator
        double gamma = 0;
		for (int j = 0; j < hmm->state_num; j++) {
			gamma = (alphas[i][j] * betas[i][j]) / sum;
			gammas[i][j] += gamma / number_of_observation_sequence;
			total_gammas_with_observation[i][j][observation_sequence[i] - 'A'] += gamma / number_of_observation_sequence;
		}
	}

	return;
}

void calculate_epsilon(HMM *hmm, char *observation_sequence, int length_of_observation_sequence, int number_of_observation_sequence, \
        double alphas[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE], \
        double betas[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE], \
        double epsilon[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE][MAX_STATE], \
        double total_epsilon[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE][MAX_STATE]) {
	for (int i = 0; i < length_of_observation_sequence - 1; i++) {
        //Calculate denominator
		double sum = 0;
        for (int j = 0; j < hmm->state_num; j++) {
            for (int k = 0; k < hmm->state_num; k++) {
				sum += alphas[i][j] * hmm->transition[j][k] * hmm->observation[observation_sequence[i + 1] - 'A'][k] * betas[i + 1][k];
            }
        }

        //Calculate numerator
        for (int j = 0; j < hmm->state_num; j++) {
			for (int k = 0 ; k < hmm->state_num ; k++) {
				epsilon[i][j][k] = alphas[i][j] * hmm->transition[j][k] * hmm->observation[observation_sequence[i + 1] - 'A'][k] * betas[i + 1][k] / sum;
				total_epsilon[i][j][k] += epsilon[i][j][k] / number_of_observation_sequence;
			}
        }
	}

	return;
}

void updateHMM(HMM *hmm, int length_of_observation_sequence, \
        double gammas[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE], \
        double total_gammas_with_observation[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE][MAX_OBSERV], \
        double total_epsilon[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE][MAX_STATE]) {
    //Update initial in HMM
    for (int i = 0; i < hmm->state_num; i++) {
		hmm->initial[i] = gammas[0][i];
    }

    //Update transition in HMM
    double sum_n = 0 , sum_d = 0;
    for (int i = 0; i < hmm->state_num; i++) {
		for (int j = 0; j < hmm->state_num; j++) {
			for (int k = 0; k < length_of_observation_sequence - 1; k++) {
				sum_n += total_epsilon[k][i][j];
				sum_d += gammas[k][i];
			}

			hmm->transition[i][j] = sum_n / sum_d;
		}
    }

    //Update observation in HMM
    for (int i = 0 ; i < hmm->observ_num ; i++) {
		for (int j = 0 ; j < hmm->state_num ; j++) {
			for (int k = 0 ; k < length_of_observation_sequence ; k++) {
				sum_n += total_gammas_with_observation[k][j][i];
				sum_d += gammas[k][j];
			}

			hmm->observation[i][j] = sum_n / sum_d;
		}
    }

	return;
}

void Baum_Welch_algorithm(HMM *hmm, int number_of_observation_sequences, int length_of_observation_sequence, char observation_sequence[MAX_OBSERV][MAX_LENGTH_OF_OBSERVATION_SEQUENCE], int iteration) {
	for (int i = 0 ; i < iteration ; i++) {
		double alphas[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE] = {{0}};
		double betas[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE] = {{0}};
		double gammas[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE] = {{0}};
		double epsilon[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE][MAX_STATE] = {{0}};

		double total_gammas_with_observation[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE][MAX_OBSERV] = {{{0}}};
		double total_epsilon[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE][MAX_STATE] = {{{0}}};

		for (int j = 0 ; j < number_of_observation_sequences ; j++) {
			forward_algorithm(hmm, observation_sequence[j], length_of_observation_sequence, alphas);
			backward_algorithm(hmm , observation_sequence[j], length_of_observation_sequence, betas);
			calculate_gamma(hmm, observation_sequence[j], length_of_observation_sequence, number_of_observation_sequences, alphas, betas, gammas, total_gammas_with_observation);
			calculate_epsilon(hmm, observation_sequence[j], length_of_observation_sequence, number_of_observation_sequences, alphas, betas, epsilon, total_epsilon);
		}

		updateHMM(hmm , length_of_observation_sequence , gammas , total_gammas_with_observation , total_epsilon);
	}

	return;
}

void trainHMM(HMM *hmm, char *file_name, int iteration) {
	FILE *training_data = open_or_die(file_name ,"r");
	char observation_sequences[MAX_NUMBER_OF_OBSERVATION_SEQUENCES][MAX_LENGTH_OF_OBSERVATION_SEQUENCE] = {{0}};

	int number_of_observation_sequences = 0;
	while(fscanf(training_data, "%s", observation_sequences[number_of_observation_sequences]) != EOF)
		number_of_observation_sequences++;

	int length_of_observation_sequence = strlen(observation_sequences[0]);
	fclose(training_data);

	Baum_Welch_algorithm(hmm , number_of_observation_sequences , length_of_observation_sequence , observation_sequences, iteration);

	return;
}


int main(int argc , char **argv) {
    // Parsing parameters
    int iteration = atoi(argv[1]);
    char *init_file = argv[2];
    char *train_data = argv[3];
    char *result_file = argv[4];

    // Create HMM model
	HMM hmm = {.state_num = 0, .observ_num = 0, .initial = {0}, .transition = {{0}}, .observation = {{0}}};

    // Initial HMM model
	loadHMM(&hmm , init_file);

    // Train HMM model
    trainHMM(&hmm, train_data, iteration);
    
    // Dump HMM model
	FILE *dump_file = open_or_die(result_file, "w");
	dumpHMM(dump_file , &hmm);
	fclose(dump_file);

    return 0;
}

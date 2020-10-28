#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/hmm.h"

#define MAX_NUMBER_OF_OBSERVATION_SEQUENCES 10001
#define MAX_LENGTH_OF_OBSERVATION_SEQUENCE 51

void forward_algorithm(HMM *hmm, char *observation_sequences, int length_of_observation_sequence, \
        double alpha[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE]) {
    // Initial the alphas with given values
    for (int i = 0; i < hmm->state_num; i++) {
		alpha[0][i] = hmm->initial[i] * hmm->observation[observation_sequences[0] - 'A'][i];
    }

    // Acumulate each alpha with the equation inside the slides
    for (int i = 1; i < length_of_observation_sequence; i++) {
		for (int j = 0; j < hmm->state_num; j++) {
			alpha[i][j] = 0;
            for (int k = 0; k < hmm->state_num; k++) {
				alpha[i][j] += alpha[i - 1][k] * hmm->transition[k][j] * hmm->observation[observation_sequences[i] - 'A'][j];
            }
		}
    }

	return;
}

void backward_algorithm(HMM *hmm, char *observation_sequences, int length_of_observation_sequence, \
        double beta[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE]) {
    // Initial the betas with given values 
    for (int i = 0; i < hmm->state_num; i++) {
		beta[length_of_observation_sequence - 1][i] = 1;
    }

    // Acumulate each beta with the equation inside the slides
    for (int i = length_of_observation_sequence - 2; i >= 0; i--) {
		for (int j = 0; j < hmm->state_num; j++) {
			beta[i][j] = 0;
			for (int k = 0; k < hmm->state_num; k++)
				beta[i][j] += hmm->transition[j][k] * hmm->observation[observation_sequences[i + 1] - 'A'][k] * beta[i + 1][k];
		}
    }

	return;
}

void calculate_gamma(HMM *hmm, char *observation_sequences, int length_of_observation_sequence, int number_of_observation_sequences, \
        double alpha[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE], \
        double beta[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE], \
        double gamma[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE], \
        double total_gamma_with_observation[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE][MAX_OBSERV]) {
	for (int i = 0; i < length_of_observation_sequence; i++) {
        //Calculate denominator
		double sum = 0;
        for (int j = 0; j < hmm->state_num; j++) {
			sum += alpha[i][j] * beta[i][j];
        }

        //Calculate numerator
		for (int j = 0; j < hmm->state_num; j++) {
			double temp_gamma = alpha[i][j] * beta[i][j] / sum;
			gamma[i][j] += temp_gamma / number_of_observation_sequences;
			total_gamma_with_observation[i][j][observation_sequences[i] - 'A'] += temp_gamma / number_of_observation_sequences;
		}
	}

	return;
}

void calculate_epsilon(HMM *hmm, char *observation_sequences, int length_of_observation_sequence, int number_of_observation_sequences, \
        double alpha[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE], \
        double beta[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE], \
        double epsilon[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE][MAX_STATE], \
        double total_epsilon[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE][MAX_STATE]) {
	for (int i = 0; i < length_of_observation_sequence - 1; i++) {
        //Calculate denominator
		double sum = 0;
        for (int j = 0; j < hmm->state_num; j++) {
            for (int k = 0; k < hmm->state_num; k++) {
				sum += alpha[i][j] * hmm->transition[j][k] * hmm->observation[observation_sequences[i + 1] - 'A'][k] * beta[i + 1][k];
            }
        }

        //Calculate numerator
        for (int j = 0; j < hmm->state_num; j++) {
			for (int k = 0; k < hmm->state_num; k++) {
				epsilon[i][j][k] = alpha[i][j] * hmm->transition[j][k] * hmm->observation[observation_sequences[i + 1] - 'A'][k] * beta[i + 1][k] / sum;
				total_epsilon[i][j][k] += epsilon[i][j][k] / number_of_observation_sequences;
			}
        }
	}

	return;
}

void updateHMM(HMM *hmm, int length_of_observation_sequence, \
        double gamma[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE], \
        double total_gamma_with_observation[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE][MAX_OBSERV], \
        double total_epsilon[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE][MAX_STATE]) {
    //Update initial in HMM
    for (int i = 0; i < hmm->state_num; i++) {
		hmm->initial[i] = gamma[0][i];
    }

    //Update transition in HMM
    for (int i = 0; i < hmm->state_num; i++) {
		for (int j = 0; j < hmm->state_num; j++) {
			double sum_n = 0, sum_d = 0;
			for (int k = 0; k < length_of_observation_sequence - 1; k++) {
				sum_n += total_epsilon[k][i][j];
				sum_d += gamma[k][i];
			}

			hmm->transition[i][j] = sum_n / sum_d;
		}
    }

    //Update observation in HMM
    for (int i = 0; i < hmm->observ_num; i++) {
		for (int j = 0; j < hmm->state_num; j++) {
			double sum_n = 0, sum_d = 0;
			for (int k = 0; k < length_of_observation_sequence; k++) {
				sum_n += total_gamma_with_observation[k][j][i];
				sum_d += gamma[k][j];
			}

			hmm->observation[i][j] = sum_n / sum_d;
		}
    }

	return;
}

void Baum_Welch_algorithm(HMM *hmm, int number_of_observation_sequences, int length_of_observation_sequence, \
        char observation_sequences[MAX_OBSERV][MAX_LENGTH_OF_OBSERVATION_SEQUENCE], int iteration) {
	for (int i = 0; i < iteration; i++) {
		double alpha[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE] = {{0}};
		double beta[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE] = {{0}};
		double gamma[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE] = {{0}};
		double epsilon[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE][MAX_STATE] = {{{0}}};
		double total_gamma_with_observation[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE][MAX_OBSERV] = {{{0}}};
		double total_epsilon[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE][MAX_STATE] = {{{0}}};

		for (int j = 0; j < number_of_observation_sequences; j++) {
			forward_algorithm(hmm, observation_sequences[j], length_of_observation_sequence, alpha);
			backward_algorithm(hmm, observation_sequences[j], length_of_observation_sequence, beta);
			calculate_gamma(hmm, observation_sequences[j], length_of_observation_sequence, number_of_observation_sequences, alpha, beta, gamma, total_gamma_with_observation);
			calculate_epsilon(hmm, observation_sequences[j], length_of_observation_sequence, number_of_observation_sequences, alpha, beta, epsilon, total_epsilon);
		}

		updateHMM(hmm, length_of_observation_sequence, gamma, total_gamma_with_observation, total_epsilon);
	}

	return;
}

void trainHMM(HMM *hmm, char *file_name, int iteration) {
	FILE *training_data = open_or_die(file_name, "r");
	char observation_sequences[MAX_NUMBER_OF_OBSERVATION_SEQUENCES][MAX_LENGTH_OF_OBSERVATION_SEQUENCE] = {{0}};

	int number_of_observation_sequences = 0;
    while (fscanf(training_data, "%s", observation_sequences[number_of_observation_sequences]) != EOF) {
		number_of_observation_sequences++;
    }
	int length_of_observation_sequence = strlen(observation_sequences[0]);
	fclose(training_data);

	Baum_Welch_algorithm(hmm, number_of_observation_sequences, length_of_observation_sequence, observation_sequences, iteration);

	return;
}

int main(int argc, char **argv) {
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

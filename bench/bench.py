#!/usr/bin/env python3

import matplotlib.pyplot as plt
import numpy as np
import json

def read_json_results(json_path, only_plot_keys):
    '''
    Read the results from a single JSON file.
    '''
    with open(json_path) as f:
        j_data = json.load(f)
    cost_info = {}
    for k,v in j_data.items():
        if len(only_plot_keys) == 0 or k in only_plot_keys:
            cost_info[k] = {
                    'mean': np.mean(v['costs']),
                    'std': np.std(v['costs']),
                    'best': np.min(v['costs']),
                    'its_mean': np.mean(v['iterations']),
                    'its_std': np.std(v['iterations'])
                    }
            print(f'{k}: best={cost_info[k]["best"]}, mean={cost_info[k]["mean"]}, '
                    f'std={cost_info[k]["std"]}')
    return cost_info

def plot_results(json_path_dicts, only_plot_keys=[]):
    '''
    Plot multiple results contained in multiple JSON paths.
    '''
    # read all results
    cost_info_dicts = {}
    for k,v in json_path_dicts.items():
        print(f'\nReading cost information for benchmark: {k}')
        results = read_json_results(v, only_plot_keys)
        cost_info_dicts[k] = results

    n_sets = len(cost_info_dicts)
    fig, ax1 = plt.subplots()
    ax2 = ax1.twinx()

    base_values = []
    base_iter_means = []
    for (ind, (set_name, cost_info)) in enumerate(cost_info_dicts.items()):
        labels = cost_info.keys()
        values = np.asarray([v['mean'] for v in cost_info.values()])
        error = [v['std'] for v in cost_info.values()]
        iter_means = np.asarray([v['its_mean'] for v in cost_info.values()])
        x = np.arange(len(labels))
        width = 0.7 # the width occupied by all bars for each category

        if ind == 0:
            base_values = values
            base_iter_means = iter_means
        else:
            rects = ax1.bar(x-(width/(n_sets-1)) + ind*width/(n_sets-1), np.divide(values-base_values, base_values), width/(n_sets-1), color=f'C{ind-1}', label=f'{set_name} relative cost')
            iters = ax2.bar(x-(width/(n_sets-1)) + ind*width/(n_sets-1), np.divide(iter_means-base_iter_means, base_iter_means), width/(n_sets-1)/10, color=f'C{ind+n_sets-1}')
            iters_dot = ax2.scatter(x-(width/(n_sets-1)) + ind*width/(n_sets-1), np.divide(iter_means-base_iter_means, base_iter_means), marker='x', label=f'{set_name} iterations', color=f'C{ind+n_sets-1}')

    ylabel = 'Cost / Default Cost'
    title = 'Placement Benchmark Costs -- Difference from Default Settings'
    ax1.set_ylabel(ylabel)
    ax1.set_title(title)
    ax1.legend(loc=4)
    ax1.grid(axis='y')
    ax1.set_xticks(x)
    ax1.set_xticklabels(labels, rotation=45)
    ax1.set_ylim(-.07, .07)
    ax2.set_ylabel('Iterations / Default Iterations')
    ax2.set_ylim(-7, 7)
    ax2.legend(loc=1)
    
    fig.tight_layout()
    plt.show()

if __name__ == '__main__':
    #plot_results({'Dynamic': './test_benches_dynamic.json',
    #    'ExpDecay': './test_benches_expdecay.json'})
    plot_results({'Dynamic': './test_benches_dynamic_sf25.json',
        'Dynamic w/o RW': './test_benches_dynamic_sf25_no_rw.json',
        'ExpDecay': './test_benches_expdecay_sf25.json',
        'ExpDecay w/o RW': './test_benches_expdecay_sf25_no_rw.json'})
        #'ExpDecayMock': './test_benches_expdecaymock.json'})

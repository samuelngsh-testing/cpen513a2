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
                    'best': np.min(v['costs'])
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
    fig, ax = plt.subplots()

    for (ind, (set_name, cost_info)) in enumerate(cost_info_dicts.items()):
        labels = cost_info.keys()
        values = [v['mean'] for v in cost_info.values()]
        error = [v['std'] for v in cost_info.values()]
        x = np.arange(len(labels))
        width = 0.35 # the width of the bars

        rects = ax.bar(x-width/2 + ind*width, values, width, label=set_name, yerr=error)

    ax.set_ylabel('Costs')
    ax.set_title('Placement Benchmark Costs')
    ax.set_xticks(x)
    ax.set_xticklabels(labels)
    ax.legend()
    
    fig.tight_layout()
    plt.show()

if __name__ == '__main__':
    #plot_results({'Dynamic': './test_benches_dynamic.json',
    #    'ExpDecay': './test_benches_expdecay.json'})
    plot_results({'Dynamic': './test_benches_dynamic_sf25.json',
        'ExpDecay': './test_benches_expdecay_sf25.json'})
        #'ExpDecayMock': './test_benches_expdecaymock.json'})

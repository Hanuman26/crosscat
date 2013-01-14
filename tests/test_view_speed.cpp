#include <iostream>

#include "Cluster.h"
#include "utils.h"
#include "numerics.h"
#include "View.h"
#include "RandomNumberGenerator.h"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>

typedef boost::numeric::ublas::matrix<double> matrixD;
using namespace std;
typedef vector<Cluster*> vectorCp;
typedef set<Cluster*> setCp;
typedef map<int, Cluster*> mapICp;
typedef setCp::iterator setCp_it;
typedef mapICp::iterator mapICp_it;
typedef vector<int>::iterator vectorI_it;

void print_cluster_memberships(View& v) {
  cout << "Cluster memberships" << endl;
  setCp_it it = v.clusters.begin();
  for(; it!=v.clusters.end(); it++) {
    Cluster &cd = **it;
    cout << cd.get_row_indices() << endl;
  }
  cout << "num clusters: " << v.get_num_clusters() << endl;
}

void print_with_header(View &v, string header) {
  cout << endl;
  cout << "=================================" << endl;
  cout << header << endl;
  v.print();
  cout << header << endl;
  cout << "=================================" << endl;
  cout << endl;
}

void remove_all_data(View &v, map<int, vector<double> > data_map) {
  vector<int> rows_in_view;
  for(mapICp_it it=v.cluster_lookup.begin(); it!=v.cluster_lookup.end(); it++) {
    rows_in_view.push_back(it->first);
  }
  for(vectorI_it it=rows_in_view.begin(); it!=rows_in_view.end(); it++) {
    int idx_to_remove = *it;
    vector<double> row = data_map[idx_to_remove];
    vector<int> global_indices = create_sequence(row.size());
    vector<double> aligned_row = v.align_data(row, global_indices);
    v.remove_row(aligned_row, idx_to_remove);
  }
  cout << "removed all data" << endl;
  v.print();
  //
  for(setCp_it it=v.clusters.begin(); it!=v.clusters.end(); it++) {
    v.remove_if_empty(**it);
  }
  assert(v.get_num_vectors()==0);
  assert(v.get_num_clusters()==0);
  cout << "removed empty clusters" << endl; 
  v.print();
}

int main(int argc, char** argv) {
  cout << endl << "Hello World!" << endl;

  // load some data
  matrixD data;
  LoadData("SynData2.csv", data);
  int num_cols = data.size2();
  int num_rows = data.size1();
  //
  map<int, vector<double> > data_map;
  cout << "populating data_map" << endl;
  for(int row_idx=0; row_idx<num_rows; row_idx++) {
    data_map[row_idx] = extract_row(data, row_idx);
  }

  // create the view
  vector<int> global_row_indices = create_sequence(data.size1());
  vector<int> global_column_indices = create_sequence(data.size2());
  View v = View(data, global_row_indices, global_column_indices, 31);

  v.print();
  // empty object and verify empty
  remove_all_data(v, data_map);
  v.print();

  // print the initial view
  print_with_header(v, "empty view print");


  cout << "insert a single row (1):";
  int row_idx = 0;
  vector<double> row = extract_row(data, row_idx);
  v.insert_row(row, row_idx);
  print_with_header(v, "view after inserting single row (1)");

  cout << "remove a single row (1):";
  row_idx = 0;
  row = extract_row(data, row_idx);
  v.remove_row(row, row_idx);
  print_with_header(v, "view after removeing single row (1)");

  cout << "printing cluster row_indices" << endl;
  for(int cluster_idx=0; cluster_idx<v.get_num_clusters(); cluster_idx++) {
    Cluster c = v.get_cluster(cluster_idx);
    cout << "cluster has row_indices: " << c.get_row_indices() << endl;
  }
  cout << "DONE printing cluster row_indices" << endl;

  // populate the objects to test
  cout << endl << "populating objects" << endl;
  cout << "=================================" << endl;
  cout << "Insertings row:";
  for(int row_idx=0; row_idx<num_rows; row_idx++) {
    cout << " " << row_idx;
    vector<double> row = extract_row(data, row_idx);
    v.insert_row(row, row_idx);
  }
  print_with_header(v, "view after population");

  cout << "====================" << endl;
  cout << "Sampling" << endl;
  // test transition
  RandomNumberGenerator rng = RandomNumberGenerator();
  for(int iter=0; iter<21; iter++) {
    v.assert_state_consistency();
    v.transition_zs(data_map);
    v.transition_crp_alpha();
    v.transition_hypers();
    if(iter % 10 == 0) {
      cout << "Done iter: " << iter << endl;
      print_with_header(v, "view after iter");
    }
  }
  cout << "Done transition_zs" << endl;
  cout << endl;
  v.print();
  v.print_score_matrix();
  cout << "v.global_to_local: " << v.global_to_local << endl;

  int remove_col_idx, insert_col_idx;
  remove_col_idx = 2;
  insert_col_idx = 2;
  vector<double> col_data = extract_col(data, insert_col_idx);
  vector<int> data_global_row_indices = create_sequence(col_data.size(), 0);
  double score_delta_1, score_delta_2, score_0, score_1;

  cout << "=====================" << endl;
  cout << "=====================" << endl;
  cout << "=====================" << endl;
  remove_col_idx = 2;
  cout << "removing column: " << remove_col_idx;
  v.remove_col(remove_col_idx);
  cout << "FLAG:: score: " << v.get_score() << endl;
  v.print_score_matrix();
  cout << "v.global_to_local: " << v.global_to_local << endl;

  cout << "=====================" << endl;
  cout << "=====================" << endl;
  cout << "=====================" << endl;
  insert_col_idx = remove_col_idx;
  cout << "inserting column: " << insert_col_idx;
  score_0 = v.get_score();
  score_delta_1 = v.score_col_data(col_data, data_global_row_indices);
  score_delta_2 = v.insert_col(col_data, data_global_row_indices, insert_col_idx);
  score_1 = v.get_score();
  cout << "FLAG:: " << "score_0: " << score_0 << ", score_1: " << score_1;
  cout << ", score_delta_1: " << score_delta_1 << ", score_delta_2: " << score_delta_2 << endl;

  v.print_score_matrix();
  cout << "v.global_to_local: " << v.global_to_local << endl;

  cout << "=====================" << endl;
  cout << "=====================" << endl;
  cout << "=====================" << endl;
  remove_col_idx = 2;
  cout << "removing column: " << remove_col_idx;
  v.remove_col(remove_col_idx);
  cout << "FLAG:: score: " << v.get_score() << endl;
  v.print_score_matrix();
  cout << "v.global_to_local: " << v.global_to_local << endl;

  cout << "=====================" << endl;
  cout << "=====================" << endl;
  cout << "=====================" << endl;
  insert_col_idx = 2;
  cout << "inserting column: " << insert_col_idx;
  v.insert_col(col_data, data_global_row_indices, insert_col_idx);
  cout << "FLAG:: score: " << v.get_score() << endl;
  v.print_score_matrix();
  cout << "v.global_to_local: " << v.global_to_local << endl;

  v.print();
  // empty object and verify empty
  remove_all_data(v, data_map);
  v.print();

  cout << "insert a single row (2): " << flush;
  row_idx = 0;
  row = extract_row(data, row_idx);

  vector<int> global_indices = create_sequence(row.size());
  vector<double> aligned_row = v.align_data(row, global_indices);

  cout << aligned_row << endl << flush;
  v.insert_row(aligned_row, row_idx);
  print_with_header(v, "view after inserting single row (2)");

  cout << "remove a single row (2):";
  v.remove_row(aligned_row, row_idx);
  print_with_header(v, "view after removeing single row (2)");

  cout << endl << "Goodbye World!" << endl;
}
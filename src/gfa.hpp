#ifndef VG_GFA_HPP_INCLUDED
#define VG_GFA_HPP_INCLUDED

/**
 * \file gfa.hpp
 *
 * Defines GFA I/O algorithms for PathHandleGraphs graphs.
 *
 * Includes an algorithm for converting from GFA, including non-perfect-match
 * edge overlaps and edges that specify containment of one node in another, to
 * a blunt-ended VG.
 */

#include "handle.hpp"
#include "snarls.hpp"
#include "traversal_finder.hpp"

namespace vg {

using namespace std;

/// Export the given VG graph to the given GFA file.
/// Express paths mentioned in rgfa_paths as rGFA.
/// If rgfa_pline is set, also express them as dedicated lines.
/// If use_w_lines is set, reference and haplotype paths will use W lines instead of P lines.
void graph_to_gfa(const PathHandleGraph* graph, ostream& out,
                  const set<string>& rgfa_paths = {},
                  bool rgfa_pline = false,
                  bool use_w_lines = true);


/// Prototype code to tag paths as rGFA paths. Either needs to be completely scrapped
/// or adapted into libhandlegraph  at some point, ideally.

/// It works by using a special sample name (default=_rGFA_) for rGFA contigs.
/// Any real sample name gets pushed into the locus field behind its rGFA tag SN:Z:<name>
/// The rGFA rank also goes in the locus field behind SR:i:<rank>

/// In GFA, these paths live in rGFA tags on S elements
/// In the graph, they are reference paths with SN/SR fields in their locus names.
/// As it stands, they will come out as W-lines in GFA with vg view or vg convert (without -Q/-P)

/// Note that rank-0 rGFA fragments (aka normal reference paths) do *not* get the rGFA
/// sample, and are treated as normal reference paths all the way through (but can get rGFA tags)
/// when specified with -Q/-P in convert -f.

/// Returns the RGFA rank (SR) of a path. This will be 0 for the reference
/// backbone, and higher the further number of (nested) bubbles away it is.
/// If the path is not an RGFA path, then return -1
int get_rgfa_rank(const string& path_name);

/// Remove the rGFA information from a path name, effectively undoing set_rgfa_rank
string strip_rgfa_path_name(const string& path_name);

/// Clamp a path to given subrange (taking into account an existing subrange)
void clamp_path_subrange(string& path_name, int64_t start, int64_t end);

/// Add the rgfa rank to a pathname, also setting its sample to the special rgfa sample and
/// moving its old sample into the locus field
string create_rgfa_path_name(const string& path_name, int rgfa_rank, const subrange_t& subrange,
                             const string& rgfa_sample,
                             const string& start_parent_path_name = "",
                             int64_t start_parent_offset = -1,
                             int64_t start_parent_node_id = -1,
                             bool start_parent_node_reversed = false,
                             const string& end_parent_path_name = "",
                             int64_t end_parent_offset = -1,
                             int64_t end_parent_node_id = -1,
                             bool end_parent_node_reversed = false);


/// Parse (and remove) all rgfa-specific information, copying it into the various input paramters
/// The rgfa-information-free vg path name is returned (ie the path_name input to carete_rgfa_path_name)
string parse_rgfa_path_name(const string& path_name, int* rgfa_rank = nullptr,
                            string* rgfa_sample = nullptr,
                            string* start_parent_rgfa_path_name = nullptr,
                            int64_t*  start_parent_offset = nullptr,
                            int64_t* start_parent_node_id = nullptr,
                            bool* start_parent_node_reversed = nullptr,
                            string* end_parent_rgfa_path_name = nullptr,
                            int64_t* end_parent_offset = nullptr,
                            int64_t* end_parent_node_id = nullptr,
                            bool* end_parent_node_reversed = nullptr);

/// Like above, but put the rgfa stuff into tags
string parse_rgfa_name_into_tags(const string& path_name, string& rgfa_tags);


/// Compute the rGFA path cover
/// graph: the graph
/// snarl_manager: the snarls (todo: should use distance index)
/// reference_paths: rank-0 paths
/// minimum_length: the minimum length of a path to create (alleles shorter than this can be uncovered)
/// preferred_intervals: set of ranges (ex from minigraph) to use as possible for rGFA paths
void rgfa_graph_cover(MutablePathMutableHandleGraph* graph,
                      SnarlManager* snarl_manager,
                      const unordered_set<path_handle_t>& reference_paths,
                      int64_t minimum_length,
                      const string& rgfa_sample_name,
                      const unordered_map<string, vector<pair<int64_t, int64_t>>>& preferred_intervals = {});

/// Some information about an RGFA fragment we pass around and index below
struct RGFAFragment {
   int64_t rank;
   vector<step_handle_t> steps;
   int64_t length;
   int64_t start_parent_idx;
   int64_t end_parent_idx;
   step_handle_t start_parent_step;
   step_handle_t end_parent_step;   
};

void rgfa_snarl_cover(const PathHandleGraph* graph,
                      const Snarl& snarl,
                      PathTraversalFinder& path_trav_finder,
                      const unordered_set<path_handle_t>& reference_paths,
                      int64_t minimum_length,
                      vector<RGFAFragment>& cover_fragments,
                      unordered_map<nid_t, int64_t>& cover_node_to_fragment,
                      const unordered_map<string, vector<pair<int64_t, int64_t>>>& preferred_intervals);

/// Get some statistics from a traversal fragment that we use for ranking in the greedy algorithm
/// 1. Coverage : total step length across the traversal for all paths
/// 2. Switches : the number of nodes that would need to be flipped to forwardize the traversal
/// 3. Duplicated bases : the number of duplicated bases in the traversal path
tuple<int64_t, int64_t, int64_t> rgfa_traversal_stats(const PathHandleGraph* graph,
                                                      const vector<step_handle_t>& trav,
                                                      const pair<int64_t, int64_t>& trav_fragment);

/// Comparison of the above stats for the purposes of greedily selecting (highest better) traversals
bool rgfa_traversal_stats_less(const tuple<int64_t, int64_t, int64_t>& s1, const tuple<int64_t, int64_t, int64_t>& s2);

/// Make sure all rgfa paths are forwardized
void rgfa_forwardize_paths(MutablePathMutableHandleGraph* graph,
                           const unordered_set<path_handle_t>& reference_paths);
                                                  
/// Extract rGFA tags from minigraph GFA in order to pass in as hints above
unordered_map<const string&, vector<pair<int64_t, int64_t>>>  extract_rgfa_intervals(const string& rgfa_path);


}

#endif

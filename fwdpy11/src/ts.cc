#include <algorithm>
#include <vector>
#include <stdexcept>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include <fwdpp/ts/table_collection.hpp>
#include <fwdpp/ts/table_simplifier.hpp>

#include <fwdpy11/types/Population.hpp>
#include <fwdpy11/rng.hpp>

namespace py = pybind11;

PYBIND11_MAKE_OPAQUE(fwdpp::ts::edge_vector);
PYBIND11_MAKE_OPAQUE(fwdpp::ts::node_vector);
PYBIND11_MAKE_OPAQUE(fwdpp::ts::mutation_key_vector);

py::tuple
simplify(const fwdpy11::Population& pop,
         const std::vector<fwdpp::ts::TS_NODE_INT>& samples)
{
    if (pop.tables.L == std::numeric_limits<double>::max())
        {
            throw std::invalid_argument(
                "population is not using tree sequences");
        }
    if (pop.tables.num_nodes() == 0)
        {
            throw std::invalid_argument(
                "population has empty TableCollection");
        }
    if (samples.empty())
        {
            throw std::invalid_argument("empty sample list");
        }
    if (std::any_of(samples.begin(), samples.end(),
                    [&pop](const fwdpp::ts::TS_NODE_INT s) {
                        return s == fwdpp::ts::TS_NULL_NODE
                               || static_cast<std::size_t>(s)
                                      >= pop.tables.num_nodes();
                    }))
        {
            throw std::invalid_argument("invalid sample list");
        }
    auto t(pop.tables);
    fwdpp::ts::table_simplifier simplifier(pop.tables.L);
    auto idmap = simplifier.simplify(t, samples, pop.mutations);
    return py::make_tuple(std::move(t), std::move(idmap));
}

PYBIND11_MODULE(ts, m)
{
    // TODO: how do I docstring this?
    m.attr("NULL_NODE") = py::int_(fwdpp::ts::TS_NULL_NODE);

    // The low-level types are numpy dtypes
    PYBIND11_NUMPY_DTYPE(fwdpp::ts::node, population, time);
    PYBIND11_NUMPY_DTYPE(fwdpp::ts::edge, left, right, parent, child);
    PYBIND11_NUMPY_DTYPE(fwdpp::ts::mutation_record, node, key);

    // The low level types are also Python classes
    py::class_<fwdpp::ts::node>(m, "Node",
                                R"delim(
            A node in a tree sequence.

            .. versionadded:: 0.2.0
            )delim")
        .def_readonly("population", &fwdpp::ts::node::population,
                      R"delim(
            For models of discrete population structure,
            this field is the population of the node.
            )delim")
        .def_readonly("time", &fwdpp::ts::node::time,
                      "Birth time of the node, recorded forwards in time.");

    py::class_<fwdpp::ts::edge>(m, "Edge",
                                R"delim(
            An edge in a tree sequence.  An edge
            represents the transmission of the
            half-open genomic interval [left,right) from
            parent to child.

            .. versionadded:: 0.2.0
            )delim")
        .def_readonly("left", &fwdpp::ts::edge::left,
                      "Left edge of interval, inclusive.")
        .def_readonly("right", &fwdpp::ts::edge::right,
                      "Right edge of interval, exclusive.")
        .def_readonly("parent", &fwdpp::ts::edge::parent, "Node id of parent")
        .def_readonly("child", &fwdpp::ts::edge::child, "Node id of child");

    py::class_<fwdpp::ts::mutation_record>(m, "MutationRecord",
                                           R"delim(
            A mutation entry in a tree sequence. A MutationRecord
            stores the node ID of the mutation and the index
            ("key") of the mutation in the population.

            .. versionadded:: 0.2.0
            )delim")
        .def_readonly("node", &fwdpp::ts::mutation_record::node,
                      "Node id of the mutation")
        .def_readonly("key", &fwdpp::ts::mutation_record::key,
                      "Index of the mutation in the population");

    // indexed_edge cannot be a dtype because it has a constructor.
    // That's probably ok, as no-one will be using them for purposes other than viewing?
    // For now, I won't even bind the C++ vector of these...
    py::class_<fwdpp::ts::indexed_edge>(
        m, "IndexedEdge",
        "An edge keyed for efficient traversal of tree sequences.")
        .def_readonly("pos", &fwdpp::ts::indexed_edge::pos)
        .def_readonly("time", &fwdpp::ts::indexed_edge::time)
        .def_readonly("parent", &fwdpp::ts::indexed_edge::parent)
        .def_readonly("child", &fwdpp::ts::indexed_edge::child);

    // The tables are visible w/o copy via numpy
    py::bind_vector<fwdpp::ts::edge_vector>(
        m, "EdgeTable", py::buffer_protocol(), py::module_local(false),
        R"delim(
        An EdgeTable is a container of :class:`fwdpy11.ts.Edge`.

        An EdgeTable supports the Python buffer protocol, allowing data
        to be viewed in Python as a numpy record array.  No copy is made
        when generating such views.

        .. versionadded:: 0.2.0
        )delim");

    py::bind_vector<fwdpp::ts::node_vector>(
        m, "NodeTable", py::buffer_protocol(), py::module_local(false),
        R"delim(
        An NodeTable is a container of :class:`fwdpy11.ts.Node`.

        An NodeTable supports the Python buffer protocol, allowing data
        to be viewed in Python as a numpy record array.  No copy is made
        when generating such views.

        .. versionadded:: 0.2.0
        )delim");

    py::bind_vector<fwdpp::ts::mutation_key_vector>(
        m, "MutationTable", py::buffer_protocol(), py::module_local(false),
        R"delim(
        An MutationTable is a container of :class:`fwdpy11.ts.MutationRecord`.

        An MutationTable supports the Python buffer protocol, allowing data
        to be viewed in Python as a numpy record array.  No copy is made
        when generating such views.

        .. versionadded:: 0.2.0
        )delim");

    // A table_collection will not be user-constructible.  Rather,
    // they will be members of the Population classes.
    // TODO: work out conversion to msprime format
    // TODO: allow preserved_nodes to be cleared
    // TODO: allow access to the "right" member functions
    py::class_<fwdpp::ts::table_collection>(m, "TableCollection")
        .def_readonly("L", &fwdpp::ts::table_collection::L, "Genome length")
        .def_readonly("edges", &fwdpp::ts::table_collection::edge_table,
                      "The :class:`fwdpy11.ts.EdgeTable`.")
        .def_readonly("nodes", &fwdpp::ts::table_collection::node_table,
                      "The :class:`fwdpy11.ts.NodeTable`.")
        .def_readonly("mutations",
                      &fwdpp::ts::table_collection::mutation_table,
                      "The :class:`fwdpy11.ts.MutationTable`.")
        .def_readonly("input_left", &fwdpp::ts::table_collection::input_left)
        .def_readonly("output_right",
                      &fwdpp::ts::table_collection::output_right)
        .def_readonly("preserved_nodes",
                      &fwdpp::ts::table_collection::preserved_nodes,
                      "List of nodes corresponding to ancient samples.");

    m.def("simplify", &simplify, py::arg("pop"), py::arg("samples"),
          R"delim(
            Simplify a TableCollection.

            :param pop: A :class:`fwdpy11.Population`
            :param samples: A list of samples (node indexes).
                
            :return: The simplified tables and list mapping input sample IDs to output IDS

            :rtype: :class:`fwdpy11.ts.TableCollection`

            Note that the samples argument is agnostic with respect to the time of
            the nodes in the input tables. Thus, you may do things like simplify
            to a set of "currently-alive" nodes plus some or all ancient samples by
            including some node IDs from :attr:`fwdpy11.ts.TableCollection.preserved_nodes`.

            )delim");

    m.def("infinite_sites",
          [](const fwdpy11::GSLrng_t& rng, fwdpp::ts::table_collection& tables,
             const std::vector<fwdpp::ts::TS_NODE_INT>& samples,
             const double mu) {
              //TODO: implement
          });
}

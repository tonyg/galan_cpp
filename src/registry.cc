#define NDEBUG
#include "registry.h"

//////////////////////////////////////////////////////////////////////////////

string Registrable::getFullpath() const {
  Registry *parent = getParent();

  if (parent)
    return parent->getFullpath() + "/" + getLocalname();
  else
    return getLocalname();
}

//////////////////////////////////////////////////////////////////////////////

Registry *Registry::root = new Registry();

Registry::leaf_t Registry::find_leaf(string const &path, bool create_missing_nodes = false)
{
  int pos = 0;

  while (path[pos] == '/')
    pos++;

  pos = path.find('/', pos);

  IFDEBUG(cerr << "Searching " << getFullpath() << " for " << path << "..." << endl);

  if (pos == path.npos) {
    // Terminal. Return the leaf.
    leaf_t result(this, path);
    IFDEBUG(cerr << "Terminal - returning." << endl);
    return result;
  } else {
    // Nonterminal. Recurse.
    string nodename = path.substr(0, pos);

    IFDEBUG(cerr << "Nonterminal - finding child " << nodename << "." << endl);

    iterator i = children.find(nodename);
    Registrable *par = (i == children.end() ? 0 : (*i).second);

    if (!par) {
      if (create_missing_nodes) {
	IFDEBUG(cerr << "Creating missing node " << nodename << endl);
	par = new Registry();
	par->parent = this;
	par->localname = nodename;
	children[nodename] = par;
      } else {
	IFDEBUG(cerr << "Intermediate node " << (getFullpath() + "/" + nodename)
		<< " not found." << endl);
	return leaf_t(0, "");
      }
    }

    Registry *par_reg = dynamic_cast<Registry *>(par);

    if (!par_reg) {
      IFDEBUG(cerr << "Intermediate node " << par->getFullpath() << " not a Registry." << endl);
      return leaf_t(0, "");
    }

    return par_reg->find_leaf(path.substr(pos + 1, path.npos), create_missing_nodes);
  }
}

bool Registry::bind(string const &path, Registrable *what, bool override = false) {
  leaf_t leaf = find_leaf(path, true);
  Registry *parent = leaf.first;

  if (parent == 0) {
    // Refuse, because some "intermediate" node wasn't a Registry.
    return false;
  }

  string const &child = leaf.second;
  iterator i = parent->children.find(child);
  bool present = (i != parent->children.end());
  bool result = (!present || override);

  if (present && override)
    parent->unbind((*i).second);

  if (result) {
    parent->children[child] = what;
    what->parent = parent;
    what->localname = child;
  }

  return result;
}

bool Registry::unbind(string const &path) {
  leaf_t leaf = find_leaf(path);
  Registry *parent = leaf.first;

  if (parent == 0) {
    // Refuse, because some "intermediate" node wasn't a Registry.
    return false;
  }

  string const &child = leaf.second;
  
  RETURN_VAL_UNLESS(parent, false);

  iterator i = parent->children.find(child);

  if (i == parent->children.end())
    return false;

  parent->unbind((*i).second);
  return true;
}

bool Registry::unbind(Registrable *what) {
  if (what->parent == this) {
    children.erase(what->localname);
    what->parent = 0;
    what->localname.erase();
    return true;
  } else {
    return false;
  }
}

Registrable *Registry::lookup(string const &path) {
  leaf_t leaf = find_leaf(path);
  Registry *parent = leaf.first;

  if (parent == 0) {
    // Intermediate node was not a Registry.
    return 0;
  }

  string const &child = leaf.second;

  RETURN_VAL_UNLESS(parent, 0);

  iterator i = parent->children.find(child);

  if (i == parent->children.end())
    return 0;

  return (*i).second;
}

//////////////////////////////////////////////////////////////////////////////

RegistryIterator::~RegistryIterator() {
  IFDEBUG(cerr << "Deleting RegistryIterator" << endl);
  if (nest) {
    IFDEBUG(cerr << "Deleting nest" << endl);
    delete nest;
  }
}

void RegistryIterator::assign(RegistryIterator const &from) {
  if (this == &from)
    return;

  IFDEBUG(cerr << "Cloning RegistryIterator " << &from << " as " << this << endl);

  position = from.position;
  end = from.end;

  if (from.nest) {
    IFDEBUG(cerr << "Cloning nest..." << endl);
    nest = new RegistryIterator();
    nest->assign(*from.nest);
  } else
    nest = 0;
}

void RegistryIterator::assign(Registry::iterator const &from, Registry::iterator const &_end) {
  IFDEBUG(cerr << "Assigning RegistryIterator" << endl);
  if (nest) {
    IFDEBUG(cerr << "  Trashing current nest" << endl);
    delete nest;
    nest = 0;
  }

  position = from;
  end = _end;

  if (position != end) {
    current = dynamic_cast<Registry *>((*position).second);

    if (current) {
      IFDEBUG(cerr << "  Building new nest" << endl);
      nest = new RegistryIterator(current->begin(), current->end());
    }
  } else {
    current = 0;
  }
}

bool RegistryIterator::operator==(RegistryIterator const &other) {
  if (nest)
    return (other.nest ? (*nest) == (*other.nest) : false);
  else
    return (other.nest ? false : position == other.position);
}

bool RegistryIterator::operator==(Registry::iterator const &other) {
  return (nest ? false : position == other);
}

Registrable * &RegistryIterator::operator*() {
  if (nest)
    return *(*nest);
  else
    return (*position).second;
}

RegistryIterator &RegistryIterator::operator++(int) {
  IFDEBUG(cerr << "Incrementing RegistryIterator" << endl);
  if (nest) {
    IFDEBUG(cerr << "  Recursing" << endl);
    (*nest)++;

    IFDEBUG(cerr << "  Testing" << endl);
    if ((*nest) != current->end()) {
      return *this;
    }

    IFDEBUG(cerr << "  Removing a level" << endl);
    delete nest;
    nest = 0;
  }

  IFDEBUG(cerr << "  Incrementing position" << endl);
  position++;
  assign(position, end);

  return *this;
}

"""
Bazel helpers.
"""

def private_include_copts(includes):
    """ Sets up copts to allow including this project as a third party dependency.

    From: https://github.com/bazelbuild/bazel/issues/2670

    Args:
      includes: headers to include
    Returns:
      Patched copts
    """
    copts = []
    prefix = ""

    # convert "@" to "external/" unless in the main workspace
    repo_name = native.repository_name()
    package_name = native.package_name()
    if repo_name != "@":
        prefix = "external/{}/".format(repo_name[1:])

    for inc in includes:
        copts.append("-I{}{}/{}".format(prefix, package_name, inc))
        copts.append("-I$(GENDIR)/{}{}/{}".format(prefix, package_name, inc))

    return copts

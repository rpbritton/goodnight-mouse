from collections import namedtuple
from typing import List

Node = namedtuple("Node", "character children")


class Codes:
    def __init__(self, characters: List[int]):
        self.characters = characters

    def _generate_tree(self, amount: int) -> Node:
        # create base, empty head node
        codes = Node(-1, [])
        if amount < 1:
            return codes

        # keep track of all leaves, as those become full codes
        leaves = [codes]
        while True:
            # always use first leaf for building
            base = leaves.pop(0)
            for character in self.characters:
                # skip duplicate characters (TODO: config?)
                if base.character == character:
                    continue
                # create leaf
                code = Node(character, [])
                # record leaf
                base.children.append(code)
                leaves.append(code)
                # exit if satisified
                if len(leaves) >= amount:
                    return codes

    def _flatten_tree(self, codes: Node) -> List[List[int]]:
        # leaves (codes with no children) are just themselves
        if not codes.children:
            return [[codes.character]]

        # create a "sorted" array of each child's code segment
        children = [self._flatten_tree(child) for child in codes.children]

        # create buckets for each segment of the first, and
        # therefore greatest, child code segment
        buckets = []
        while children[0]:
            code = [codes.character, *(children[0].pop())]
            buckets.append([code])

        # add remaining child codes to bucket iteratively, first code
        index = 0
        code_exists = True
        while code_exists:
            code_exists = False
            for child in children:
                if not child:
                    continue
                buckets[index].append([codes.character, *child.pop()])
                index = (index + 1) % len(buckets)
                if child:
                    code_exists = True

        # return flattened buckets
        return [code for bucket in buckets for code in bucket]

    def generate(self, amount: int) -> List[List[int]]:
        if amount < 1:
            return []
        codes_tree = self._generate_tree(amount)
        codes_list = self._flatten_tree(codes_tree)
        return [code[1:] for code in codes_list]

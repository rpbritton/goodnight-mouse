class Codes:
    def __init__(self, characters):
        # TODO: check if characters is empty
        self.characters = characters

    def _generate_tree(self, amount):
        # create base, empty head node
        codes = ("", [])
        if amount < 1:
            return codes

        # keep track of all leaves, as those become full codes
        leaves = [codes]
        while True:
            # always use first leaf for building
            base = leaves.pop(0)
            for character in self.characters:
                # skip duplicate characters (TODO: config?)
                if base[0] == character:
                    continue
                # create leaf
                code = (character, [])
                # record leaf
                base[1].append(code)
                leaves.append(code)
                # exit it satisified
                if len(leaves) >= amount:
                    return codes

    def _flatten_tree(self, codes):
        # leaves (codes with no children) are just themselves
        if len(codes[1]) == 0:
            return [codes[0]]

        # create a "sorted" array of each child's code segment
        children = []
        for child in codes[1]:
            children.append(self._flatten_tree(child))
        
        # create buckets for each segment of the first, and
        # therefore greatest, child code segment
        buckets = [[codes[0] + code] for code in children[0]]
        # add remaining child codes to bucket iteratively, first code
        index = 0
        code_exists = True
        while code_exists:
            code_exists = False
            for child in children[1:]:
                if len(child) > 0:
                    buckets[index].append(codes[0] + child.pop(0))
                    index = (index + 1) % len(buckets)
                    if len(child) > 0:
                        code_exists = True

        # return flattened buckets
        return [code for bucket in buckets for code in bucket]

    def generate(self, amount):
        if amount < 1:
            return []
        codes = self._generate_tree(amount)
        codes = self._flatten_tree(codes)
        return codes
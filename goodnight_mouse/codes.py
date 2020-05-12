class Codes:
    def __init__(self, characters):
        # TODO: check if characters is empty
        self.characters = characters

    def _generate_codes(self, amount):
        codes = [""]
        while len(codes) < amount:
            base = codes.pop(0)
            for character in self.characters:
                codes.append(base + character)
                if len(codes) >= amount:
                    return codes
        return codes

    def _optimize_codes(self, codes):
        # sort codes by character strength
        codes.sort(key=lambda word: [self.characters.index(character) for character in word])
        # find number of codes the strongest (and implicitly most often) character starts
        num_buckets = sum(1 for code in codes if code[0] == self.characters[0])
        # create base buckets of the strongest character codes
        buckets = [[code] for code in codes[:num_buckets]]
        # add remaining codes to the ends of the buckets
        for index in range(num_buckets, len(codes)):
            buckets[index % num_buckets].append(codes[index])
        # return a flattened version of the now sorted buckets
        return [code for bucket in buckets for code in bucket]

    def generate(self, amount):
        codes = self._generate_codes(amount)
        codes = self._optimize_codes(codes)
        return codes
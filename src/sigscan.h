#define SIG_SCAN(x, y, ...)                                                                                                                                                                            \
	void *x##Addr;                                                                                                                                                                                     \
	void *x () {                                                                                                                                                                                       \
		const char *x##Data[] = {__VA_ARGS__};                                                                                                                                                         \
		size_t x##Size        = _countof (x##Data);                                                                                                                                                    \
		if (!x##Addr) {                                                                                                                                                                                \
			if (x##Size == 2) {                                                                                                                                                                        \
				x##Addr = sigScan (x##Data[0], x##Data[1], (void *)(y));                                                                                                                               \
			} else {                                                                                                                                                                                   \
				for (int i = 0; i < x##Size; i += 2) {                                                                                                                                                 \
					x##Addr = sigScan (x##Data[i], x##Data[i + 1], (void *)(y));                                                                                                                       \
					if (x##Addr) return x##Addr;                                                                                                                                                       \
				}                                                                                                                                                                                      \
			}                                                                                                                                                                                          \
		}                                                                                                                                                                                              \
		return x##Addr;                                                                                                                                                                                \
	}

void *sigScan (const char *signature, const char *mask, void *hint);
